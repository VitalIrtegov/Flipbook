#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showNextImage();
    void showPrevImage();
    void showNotesDialog();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void showWelcomeScreen();
    void updateImage();
    void updateWindowSize();
    void updateButtonPositions();
    void createProgressIndicator();
    void updateProgressIndicator();
    QString getImageSizeText(const QString &imagePath) const;
    QString loadTextFromFile(const QString &filePath) const;

    QLabel *m_imageLabel;
    QLabel *m_infoLabel;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;

    int m_currentIndex;
    QStringList m_imagePaths;
    QPixmap m_currentPixmap;
    bool m_isWelcomeScreen;
    QPushButton *m_notesButton;

    QHBoxLayout *m_progressLayout; // Layout для индикатора прогресса
    QWidget *m_progressWidget;     // Виджет для индикатора
    QList<QLabel*> m_progressLabels; // Миниатюры для прогресса
    void centerCurrentThumbnail();
};

#endif // MAINWINDOW_H
