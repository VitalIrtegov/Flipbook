#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void nextStep();
    void prevStep();
    void showNoteDialog();

private:
    void updateContent();
    void loadStepText(int stepIndex);
    QString findTextFileForImage(const QString &imagePath) const;

    QLabel *m_imageLabel;
    QLabel *m_textLabel;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QPushButton *m_noteButton;

    QStringList m_imagePaths;
    int m_currentIndex;
};

#endif // MAINWINDOW_H
