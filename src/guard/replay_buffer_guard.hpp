#pragma once

#include <QCheckBox>
#include <QObject>
#include <QPointer>

#include <memory>

class QMainWindow;
class QWidget;

class ReplayBufferGuard : public QObject {
public:
	explicit ReplayBufferGuard(QMainWindow *mainWindow);
	~ReplayBufferGuard() override;

	void attachLate();
	void prepareShutdown();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	void attachStopButton();
	void attachTrayAction();
	void injectSettingsPage(QWidget *dialog);
	void setConfirmEnabled(bool enabled, bool persist);
	void refreshControls();
	bool askConfirmation();
	void activate();

	QPointer<QMainWindow> m_mainWindow;
	QPointer<QCheckBox> m_pageToggle;
	bool m_confirmEnabled = true;
	bool m_stopButtonAttached = false;
	bool m_trayAttached = false;
	bool m_shuttingDown = false;
};