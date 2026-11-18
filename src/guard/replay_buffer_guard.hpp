#pragma once

#include <QCheckBox>
#include <QObject>
#include <QPointer>

class QAction;
class QMainWindow;
class QMenu;
class QWidget;

class ReplayBufferGuard : public QObject {
public:
	explicit ReplayBufferGuard(QMainWindow *mainWindow);
	~ReplayBufferGuard() override;

	void attachLate();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	void buildMenu();
	void attachStopButton();
	void attachTrayAction();
	void injectSettingsPage(QWidget *dialog);
	void setConfirmEnabled(bool enabled, bool persist);
	void refreshControls();
	bool askConfirmation();
	void activate();

	QMainWindow *m_mainWindow = nullptr;
	QMenu *m_menu = nullptr;
	QAction *m_menuToggle = nullptr;
	QPointer<QCheckBox> m_pageToggle;
	bool m_confirmEnabled = true;
	bool m_stopButtonAttached = false;
	bool m_trayAttached = false;
};