#include "replay_buffer_guard.hpp"
#include "settings.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QEvent>
#include <QFormLayout>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSystemTrayIcon>

#include <functional>

namespace {

constexpr const char *kSettingsClass = "OBSBasicSettings";
constexpr const char *kStopButtonName = "replayBufferButton";
constexpr const char *kAnchorName = "warnBeforeRecordStop";
constexpr int kTrayActionIndex = 7;

// 回放缓存按钮的 clicked 会直接进 OBSBasic 的槽，只能在事件到达前吃掉。
class StopButtonFilter : public QObject {
public:
	StopButtonFilter(QPushButton *button, std::function<void()> activate, QObject *parent)
		: QObject(parent), m_button(button), m_activate(std::move(activate))
	{
		m_button->installEventFilter(this);
	}

protected:
	bool eventFilter(QObject *watched, QEvent *event) override
	{
		if (watched != m_button || !obs_frontend_replay_buffer_active())
			return false;

		switch (event->type()) {
		case QEvent::MouseButtonPress: {
			auto *mouse = static_cast<QMouseEvent *>(event);
			if (mouse->button() != Qt::LeftButton)
				return false;
			m_pressed = true;
			m_button->setDown(true);
			return true;
		}
		case QEvent::MouseButtonRelease: {
			auto *mouse = static_cast<QMouseEvent *>(event);
			if (mouse->button() != Qt::LeftButton)
				return false;
			const bool inside = m_pressed && m_button->rect().contains(mouse->position().toPoint());
			m_pressed = false;
			m_button->setDown(false);
			if (inside)
				m_activate();
			return true;
		}
		case QEvent::KeyPress:
		case QEvent::KeyRelease: {
			auto *key = static_cast<QKeyEvent *>(event);
			if (key->key() != Qt::Key_Space && key->key() != Qt::Key_Return &&
			    key->key() != Qt::Key_Enter)
				return false;
			if (event->type() == QEvent::KeyRelease && !key->isAutoRepeat())
				m_activate();
			return true;
		}
		default:
			return false;
		}
	}

private:
	QPushButton *m_button = nullptr;
	bool m_pressed = false;
	std::function<void()> m_activate;
};

} // namespace

ReplayBufferGuard::ReplayBufferGuard(QMainWindow *mainWindow)
	: QObject(mainWindow),
	  m_mainWindow(mainWindow),
	  m_confirmEnabled(plugin_config::LoadConfirmEnabled())
{
	attachStopButton();
	attachTrayAction();
	qApp->installEventFilter(this);
}

ReplayBufferGuard::~ReplayBufferGuard()
{
	prepareShutdown();
	qApp->removeEventFilter(this);
}

void ReplayBufferGuard::attachLate()
{
	if (m_shuttingDown)
		return;
	attachStopButton();
	attachTrayAction();
}

// 主窗口析构期间 Qt 仍会派发信号，此时触碰界面或主窗口的动作都要停下。
void ReplayBufferGuard::prepareShutdown()
{
	m_shuttingDown = true;
}

void ReplayBufferGuard::attachStopButton()
{
	if (m_stopButtonAttached || m_shuttingDown || !m_mainWindow)
		return;

	auto *button = m_mainWindow->findChild<QPushButton *>(kStopButtonName);
	if (!button) {
		blog(LOG_WARNING, "[replay-buffer-confirm] replay buffer button not found");
		return;
	}

	new StopButtonFilter(button, [this] { activate(); }, this);
	m_stopButtonAttached = true;
}

void ReplayBufferGuard::attachTrayAction()
{
	if (m_trayAttached || m_shuttingDown || !m_mainWindow)
		return;

	auto *tray = m_mainWindow->findChild<QSystemTrayIcon *>();
	if (!tray) {
		blog(LOG_INFO, "[replay-buffer-confirm] system tray not available yet");
		return;
	}
	auto *menu = tray->contextMenu();
	if (!menu)
		return;

	QAction *target = nullptr;
	const QString stopText = QCoreApplication::translate("OBSBasic", "Basic.Main.StopReplayBuffer");
	if (!stopText.isEmpty()) {
		for (auto *action : menu->actions()) {
			if (!action->isSeparator() && action->text() == stopText) {
				target = action;
				break;
			}
		}
	}

	if (!target) {
		const auto actions = menu->actions();
		if (actions.size() > kTrayActionIndex && !actions.at(kTrayActionIndex)->isSeparator())
			target = actions.at(kTrayActionIndex);
	}

	if (!target) {
		blog(LOG_WARNING, "[replay-buffer-confirm] tray replay buffer action not found");
		return;
	}

	// 托盘项是多播信号，不断开原连接就没法在停止前插入确认。
	QObject::disconnect(target, nullptr, nullptr, nullptr);
	connect(target, &QAction::triggered, this, [this] {
		if (m_shuttingDown || !m_mainWindow)
			return;
		if (obs_frontend_replay_buffer_active())
			activate();
		else
			obs_frontend_replay_buffer_start();
	});
	m_trayAttached = true;
}

void ReplayBufferGuard::injectSettingsPage(QWidget *dialog)
{
	if (m_pageToggle && m_pageToggle->parentWidget() == dialog)
		return;

	auto *anchor = dialog->findChild<QCheckBox *>(kAnchorName);
	if (!anchor) {
		blog(LOG_WARNING, "[replay-buffer-confirm] settings anchor not found");
		return;
	}

	QWidget *container = anchor->parentWidget();
	auto *layout = container ? qobject_cast<QFormLayout *>(container->layout()) : nullptr;
	if (!layout) {
		blog(LOG_WARNING, "[replay-buffer-confirm] settings layout unexpected");
		return;
	}

	auto *toggle = new QCheckBox(obs_module_text("Confirm.PageLabel"), dialog);
	toggle->setChecked(m_confirmEnabled);
	connect(toggle, &QCheckBox::toggled, this, [this](bool on) { setConfirmEnabled(on, true); });

	int row = -1;
	for (int i = 0; i < layout->rowCount(); ++i) {
		auto *item = layout->itemAt(i, QFormLayout::FieldRole);
		if (item && item->widget() == anchor) {
			row = i;
			break;
		}
	}

	if (row >= 0)
		layout->insertRow(row + 1, QString(), toggle);
	else
		layout->addRow(QString(), toggle);

	m_pageToggle = toggle;
}

void ReplayBufferGuard::setConfirmEnabled(bool enabled, bool persist)
{
	if (m_shuttingDown)
		return;

	m_confirmEnabled = enabled;
	refreshControls();
	if (persist)
		plugin_config::SaveConfirmEnabled(enabled);
}

void ReplayBufferGuard::refreshControls()
{
	if (m_pageToggle) {
		const QSignalBlocker blocker(m_pageToggle);
		m_pageToggle->setChecked(m_confirmEnabled);
	}
}

bool ReplayBufferGuard::askConfirmation()
{
	// 标准按钮的文案来自 Qt 的翻译包，OBS 没随包附带，会显示英文 yes/no。
	QMessageBox box(m_mainWindow);
	box.setIcon(QMessageBox::Question);
	box.setWindowTitle(obs_module_text("Confirm.Title"));
	box.setText(obs_module_text("Confirm.Text"));

	auto *confirmButton = box.addButton(obs_module_text("Confirm.Yes"), QMessageBox::YesRole);
	auto *cancelButton = box.addButton(obs_module_text("Confirm.No"), QMessageBox::NoRole);
	box.setDefaultButton(cancelButton);
	box.exec();

	return box.clickedButton() == confirmButton;
}

void ReplayBufferGuard::activate()
{
	if (m_shuttingDown || !m_mainWindow)
		return;
	if (!obs_frontend_replay_buffer_active())
		return;
	if (m_confirmEnabled && m_mainWindow->isVisible() && !askConfirmation())
		return;
	obs_frontend_replay_buffer_stop();
}

bool ReplayBufferGuard::eventFilter(QObject *watched, QEvent *event)
{
	if (m_shuttingDown)
		return false;

	if (event->type() == QEvent::Show) {
		auto *widget = qobject_cast<QWidget *>(watched);
		if (widget && widget->inherits(kSettingsClass))
			injectSettingsPage(widget);
	}
	return QObject::eventFilter(watched, event);
}