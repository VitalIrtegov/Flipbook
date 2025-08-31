#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QResizeEvent>
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentIndex(0)
{
    setWindowTitle("Инструкция по сборке");

    // Запрещаем изменение размера окна
    setFixedSize(800, 600);

    // Центрируем окно на экране
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    // Загружаем изображения из папки resources
    QDir resourcesDir("resources");
    QStringList imageFilters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    m_imagePaths = resourcesDir.entryList(imageFilters, QDir::Files);
    for (QString &path : m_imagePaths) {
        path = "resources/" + path;
    }
    m_imagePaths.sort(); // Сортируем по алфавиту

    // Создаем центральный виджет
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Контейнер для изображения с кнопками навигации
    QWidget *imageContainer = new QWidget(centralWidget);
    imageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *imageLayout = new QHBoxLayout(imageContainer);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(0);

    // Создаем кнопки навигации
    m_prevButton = new QPushButton("◀", imageContainer);
    m_nextButton = new QPushButton("▶", imageContainer);

    // Стили для кнопок навигации
    QString navButtonStyle = "QPushButton {"
                             "font-size: 24pt;"
                             "background-color: rgba(255, 255, 255, 150);"
                             "border: 2px solid gray;"
                             "border-radius: 20px;"
                             "min-width: 50px;"
                             "min-height: 50px;"
                             "}"
                             "QPushButton:hover {"
                             "background-color: rgba(255, 255, 255, 200);"
                             "}"
                             "QPushButton:disabled {"
                             "background-color: rgba(200, 200, 200, 100);"
                             "}";

    m_prevButton->setStyleSheet(navButtonStyle);
    m_nextButton->setStyleSheet(navButtonStyle);
    m_prevButton->setCursor(Qt::PointingHandCursor);
    m_nextButton->setCursor(Qt::PointingHandCursor);

    // Виджет для изображения
    m_imageLabel = new QLabel(imageContainer);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_imageLabel->setStyleSheet("border: 1px solid #cccccc;");
    m_imageLabel->setMinimumSize(400, 300);

    // Размещаем кнопки поверх изображения
    imageLayout->addWidget(m_prevButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    imageLayout->addWidget(m_imageLabel, 1);
    imageLayout->addWidget(m_nextButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    // Текстовое пояснение
    m_textLabel = new QLabel(centralWidget);
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setWordWrap(true);
    m_textLabel->setStyleSheet("font-size: 12pt; padding: 10px; background-color: #f8f8f8; border: 1px solid #eeeeee; border-radius: 5px;");
    m_textLabel->setMinimumHeight(80);
    m_textLabel->setMaximumHeight(120);

    // Кнопка замечания
    m_noteButton = new QPushButton("Замечание", centralWidget);
    m_noteButton->setStyleSheet("font-size: 11pt; padding: 8px 16px;");
    m_noteButton->setCursor(Qt::PointingHandCursor);

    // Собираем основной layout
    mainLayout->addWidget(imageContainer, 4); // 4/5 места для изображения
    mainLayout->addWidget(m_textLabel, 1);    // 1/5 места для текста
    mainLayout->addWidget(m_noteButton, 0, Qt::AlignHCenter);

    setCentralWidget(centralWidget);

    // Подключаем сигналы
    connect(m_nextButton, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(m_prevButton, &QPushButton::clicked, this, &MainWindow::prevStep);
    connect(m_noteButton, &QPushButton::clicked, this, &MainWindow::showNoteDialog);

    // Показываем первый шаг
    updateContent();

}

MainWindow::~MainWindow() {}

void MainWindow::updateContent()
{
    if (m_currentIndex >= 0 && m_currentIndex < m_imagePaths.size()) {
        // Загружаем изображение
        QPixmap pixmap(m_imagePaths[m_currentIndex]);
        if (!pixmap.isNull()) {
            // Масштабируем изображение под размер виджета
            m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->width() - 10,
                                                  m_imageLabel->height() - 10,
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation));
        }

        // Загружаем текст
        loadStepText(m_currentIndex);
    }

    // Обновляем состояние кнопок навигации
    m_prevButton->setEnabled(m_currentIndex > 0);
    m_nextButton->setEnabled(m_currentIndex < m_imagePaths.size() - 1);
}

void MainWindow::loadStepText(int stepIndex)
{
    QString textFilePath = findTextFileForImage(m_imagePaths[stepIndex]);

    if (QFile::exists(textFilePath)) {
        QFile file(textFilePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString text = in.readAll();
            m_textLabel->setText(text);
            file.close();
            return;
        }
    }

    // Если файл не найден, показываем заглушку
    m_textLabel->setText(QString("Описание для шага %1\n(файл не найден)").arg(stepIndex + 1));
}

QString MainWindow::findTextFileForImage(const QString &imagePath) const
{
    QFileInfo imageInfo(imagePath);
    QString baseName = imageInfo.completeBaseName(); // Имя файла без расширения
    return QString("resources/%1.txt").arg(baseName);
}

void MainWindow::nextStep()
{
    if (m_currentIndex < m_imagePaths.size() - 1) {
        m_currentIndex++;
        updateContent();
    }
}

void MainWindow::prevStep()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        updateContent();
    }
}

void MainWindow::showNoteDialog()
{
    QMessageBox::information(this,
                             "Замечание",
                             QString("Функция замечания для шага %1\n(будет реализована позже)").arg(m_currentIndex + 1)
                             );
}
