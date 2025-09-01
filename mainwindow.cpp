#include "mainwindow.h"
#include "notesdialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QResizeEvent>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentIndex(-1) // -1 это приветственный экран
    , m_isWelcomeScreen(true)
    , m_progressWidget(nullptr)
    , m_progressLayout(nullptr)
{
    setWindowTitle("Инструкция по сборке");

    // Устанавливаем начальный размер 800x600
    resize(800, 600);

    // Центрируем окно на экране
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - 800) / 2;
        int y = (screenGeometry.height() - 600) / 2;
        move(x, y);
    }

    // Загружаем список изображений из папки resources
    QDir resourcesDir(QApplication::applicationDirPath() + "/resources");
    if (!resourcesDir.exists()) {
        qDebug() << "Resources directory does not exist!";
        // Создаем папку для демонстрации
        resourcesDir.mkpath(".");
    }

    QStringList imageFilters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    m_imagePaths = resourcesDir.entryList(imageFilters, QDir::Files);

    for (QString &path : m_imagePaths) {
        path = "resources/" + path;
    }
    m_imagePaths.sort();

    qDebug() << "Found" << m_imagePaths.size() << "images:" << m_imagePaths;

    // Создаем центральный виджет
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 5);

    // 1. Создаем индикатор прогресса БЕЗ прокрутки
    m_progressWidget = new QWidget(centralWidget);
    m_progressLayout = new QHBoxLayout(m_progressWidget);
    m_progressLayout->setSpacing(2);
    m_progressLayout->setContentsMargins(5, 5, 5, 5);

    m_progressWidget->setStyleSheet("background-color: #f0f0f0; border-bottom: 1px solid #cccccc;");
    m_progressWidget->setFixedHeight(60);
    m_progressWidget->hide(); // Сначала скрываем

    // 2. Создаем imageLabel
    m_imageLabel = new QLabel(centralWidget);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_imageLabel->setStyleSheet("border: 2px solid #cccccc; background-color: #f8f8f8;");
    m_imageLabel->setMinimumSize(100, 100);

    // 3. Создаем кнопки (СНАЧАЛА кнопки!)
    m_prevButton = new QPushButton("◀", centralWidget);
    m_nextButton = new QPushButton("▶", centralWidget);

    // Стили для кнопок
    QString navButtonStyle = "QPushButton {"
                             "font-size: 36pt;" // Увеличиваем размер стрелок
                             "background-color: transparent;" // Полностью прозрачный фон
                             "border: none;" // Убираем границу
                             "color: rgba(255, 255, 255, 200);" // Белые полупрозрачные стрелки
                             //"color: rgba(0, 0, 0, 150);" // Темные полупрозрачные стрелки
                             "min-width: 70px;" // Увеличиваем размер кнопок
                             "min-height: 70px;"
                             "}"
                             "QPushButton:hover {"
                             "color: rgba(255, 255, 255, 255);" // Белые непрозрачные при наведении
                             //"color: rgba(0, 0, 0, 200);" // Темные при наведении
                             "background-color: rgba(255, 255, 255, 30);" // Легкий фон при наведении
                             //"background-color: rgba(0, 0, 0, 20);" // Темный фон при наведении
                             "border-radius: 35px;" // Круглый фон при наведении
                             "}"
                             "QPushButton:disabled {"
                             "color: rgba(255, 255, 255, 100);" // Более прозрачные когда disabled
                             "}";

    m_prevButton->setStyleSheet(navButtonStyle);
    m_nextButton->setStyleSheet(navButtonStyle);
    m_prevButton->setCursor(Qt::PointingHandCursor);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_prevButton->setFixedSize(70, 70);
    m_nextButton->setFixedSize(70, 70);

    // 4. Создаем infoLabel
    m_infoLabel = new QLabel(centralWidget);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("font-size: 14pt; font-weight: bold; padding: 8px; background-color: #f0f0f0;");
    m_infoLabel->setMinimumHeight(60);
    m_infoLabel->setMaximumHeight(100);

    // 5. Создаем кнопку замечаний
    m_notesButton = new QPushButton("📝 Замечание", centralWidget);
    m_notesButton->setStyleSheet("font-size: 11pt; padding: 8px; background-color: #FF9800; color: white; border: none; border-radius: 5px;");
    m_notesButton->setCursor(Qt::PointingHandCursor);
    m_notesButton->setFixedHeight(35);

    // Layout для кнопки замечаний
    QHBoxLayout *notesLayout = new QHBoxLayout();
    notesLayout->addStretch();
    notesLayout->addWidget(m_notesButton);
    notesLayout->addStretch();

    // Собираем основной layout
    mainLayout->addWidget(m_progressWidget);    // Индикатор прогресса сверху
    mainLayout->addWidget(m_imageLabel, 4);     // Основное изображение
    mainLayout->addWidget(m_infoLabel, 1);      // Текст
    mainLayout->addLayout(notesLayout);         // Кнопка замечаний

    // Правильный порядок отображения (z-order)
    m_prevButton->raise();
    m_nextButton->raise();
    m_imageLabel->raise();

    setCentralWidget(centralWidget);

    // Подключаем сигналы
    connect(m_nextButton, &QPushButton::clicked, this, &MainWindow::showNextImage);
    connect(m_prevButton, &QPushButton::clicked, this, &MainWindow::showPrevImage);
    // Подключаем сигнал кнопки
    connect(m_notesButton, &QPushButton::clicked, this, &MainWindow::showNotesDialog);

    // Сначала скрываем кнопку замечаний
    m_notesButton->hide();

    // 8. Показываем приветственный экран
    showWelcomeScreen();

    // 9. Обновляем позиции кнопок
    QTimer::singleShot(100, this, &MainWindow::updateButtonPositions);
}

MainWindow::~MainWindow() { }

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Центрируем миниатюры при изменении размера окна
    if (m_progressWidget && m_progressWidget->isVisible()) {
        centerCurrentThumbnail();
    }

    updateButtonPositions();
}

void MainWindow::updateButtonPositions()
{
    if (!m_imageLabel) return;

    // Получаем геометрию imageLabel
    QRect imageRect = m_imageLabel->geometry();

    // Центрируем кнопки по вертикали в середине imageLabel
    int buttonY = imageRect.y() + (imageRect.height() - 70) / 2;

    // Левая кнопка - левый край imageLabel
    m_prevButton->move(imageRect.x(), buttonY);

    // Правая кнопка - правый край imageLabel
    m_nextButton->move(imageRect.x() + imageRect.width() - 70, buttonY);

    // ОБЕСПЕЧИВАЕМ, ЧТО КНОПКИ ВСЕГДА НАВЕРХУ
    m_prevButton->raise();
    m_nextButton->raise();

    qDebug() << "Buttons raised to top";
    qDebug() << "Prev button visible:" << m_prevButton->isVisible();
    qDebug() << "Next button visible:" << m_nextButton->isVisible();
    qDebug() << "Prev button enabled:" << m_prevButton->isEnabled();
    qDebug() << "Next button enabled:" << m_nextButton->isEnabled();
}

void MainWindow::showWelcomeScreen()
{
    m_isWelcomeScreen = true;

    // Очищаем изображение
    m_imageLabel->clear();
    m_imageLabel->setStyleSheet("border: 2px solid #cccccc; background-color: #f8f8f8; font-size: 16pt;");

    // скрываем кнопку заметки
    m_notesButton->hide();

    // Скрываем индикатор прогресса на приветственном экране
    m_progressWidget->hide();

    // Устанавливаем приветственный текст
    QString welcomeText = "Добро пожаловать в инструкцию по сборке!\n\n"
                          "Для начала просмотра нажмите кнопку \"▶\"\n\n"
                          "Используйте кнопки навигации для перемещения\n"
                          "между шагами сборки";

    m_imageLabel->setText(welcomeText);
    m_imageLabel->setAlignment(Qt::AlignCenter);

    // Устанавливаем информационный текст
    m_infoLabel->setText("Готов к работе");

    // ОБЕСПЕЧИВАЕМ ВИДИМОСТЬ КНОПОК
    m_prevButton->show();
    m_nextButton->show();
    m_prevButton->raise();
    m_nextButton->raise();

    // Обновляем состояние кнопок
    m_prevButton->setEnabled(false);
    m_nextButton->setEnabled(!m_imagePaths.isEmpty());

    // Обновляем позиции кнопок
    updateButtonPositions();

    // Устанавливаем фиксированный размер окна
    setFixedSize(800, 600);

    // Центрируем окно
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - 800) / 2;
        int y = (screenGeometry.height() - 600) / 2;
        move(x, y);
    }
}

void MainWindow::updateProgressIndicator()
{
    for (int i = 0; i < m_progressLabels.size(); ++i) {
        if (i == m_currentIndex) {
            // Выделяем текущее изображение
            m_progressLabels[i]->setStyleSheet("border: 3px solid #2196F3; background-color: #e3f2fd;");
        } else {
            // Обычный стиль для остальных
            m_progressLabels[i]->setStyleSheet("border: 2px solid #cccccc; background-color: #ffffff;");
        }
    }

    // Центрируем текущую миниатюру
    centerCurrentThumbnail();
}

void MainWindow::showNextImage()
{
    if (m_imagePaths.isEmpty()) {
        qDebug() << "No images available";
        return;
    }

    if (m_isWelcomeScreen) {
        // Переход с приветственного экрана на первый рисунок
        m_isWelcomeScreen = false;
        m_currentIndex = 0;
        updateImage();
        return;
    }

    if (m_currentIndex < m_imagePaths.size() - 1) {
        m_currentIndex++;
        updateImage();
    }

    // Обновляем индикатор прогресса
    updateProgressIndicator();

    // Обновляем состояние кнопок
    m_prevButton->setEnabled(m_currentIndex > 0);
    m_nextButton->setEnabled(m_currentIndex < m_imagePaths.size() - 1);
}

void MainWindow::showPrevImage()
{
    if (m_imagePaths.isEmpty()) {
        qDebug() << "No images available";
        return;
    }

    if (m_currentIndex == 0) {
        // Возврат к приветственному экрану
        showWelcomeScreen();
        return;
    }

    if (m_currentIndex > 0) {
        m_currentIndex--;
        updateImage();
    }

    // Обновляем индикатор прогресса
    updateProgressIndicator();

    // Обновляем состояние кнопок
    m_prevButton->setEnabled(m_currentIndex > 0);
    m_nextButton->setEnabled(m_currentIndex < m_imagePaths.size() - 1);
}

void MainWindow::updateImage()
{
    if (m_imagePaths.isEmpty()) {
        m_imageLabel->setText("Нет изображений для отображения\nДобавьте изображения в папку resources");
        m_infoLabel->setText("Папка resources пуста");
        m_progressWidget->hide(); // Скрываем индикатор если нет изображений
        return;
    }

    if (m_currentIndex < 0 || m_currentIndex >= m_imagePaths.size()) {
        m_imageLabel->setText("Ошибка: неверный индекс изображения");
        m_infoLabel->setText("Ошибка загрузки");
        return;
    }

    // ПОКАЗЫВАЕМ индикатор прогресса для изображений
    m_progressWidget->show();
    createProgressIndicator(); // Обновляем индикатор

    QString imagePath = m_imagePaths[m_currentIndex];

    // Загружаем изображение
    m_currentPixmap = QPixmap(imagePath);

    if (m_currentPixmap.isNull()) {
        m_imageLabel->setText("Не удалось загрузить изображение:\n" + imagePath);
        m_infoLabel->setText("Ошибка загрузки файла");
        return;
    }

    // Восстанавливаем стандартный стиль
    m_imageLabel->setStyleSheet("border: 2px solid #cccccc; background-color: #f8f8f8;");

    // Отображаем изображение
    m_imageLabel->setPixmap(m_currentPixmap);
    m_imageLabel->setAlignment(Qt::AlignCenter);

    // ОБЕСПЕЧИВАЕМ ВИДИМОСТЬ КНОПОК
    m_prevButton->show();
    m_nextButton->show();
    m_prevButton->raise();
    m_nextButton->raise();

    // Показываем кнопку замечаний только для изображений
    m_notesButton->setVisible(!m_isWelcomeScreen);

    // Обновляем информацию о размере
    m_infoLabel->setText(getImageSizeText(imagePath));

    // Меняем размер окна под изображение
    updateWindowSize();

    // Обновляем позиции кнопок
    updateButtonPositions();
}

void MainWindow::updateWindowSize()
{
    if (m_currentPixmap.isNull()) {
        return;
    }

    // Снимаем фиксированный размер
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    // Получаем размер изображения
    int imageWidth = m_currentPixmap.width();
    int imageHeight = m_currentPixmap.height();

    // Рассчитываем и добавляем отступы для информации и индикатора прогресса
    int infoLabelHeight = m_infoLabel->sizeHint().height();
    int progressHeight = m_progressWidget->isVisible() ? 60 : 0; // Высота индикатора
    int layoutSpacing = centralWidget()->layout()->spacing();
    int layoutMargins = centralWidget()->layout()->contentsMargins().bottom();

    int windowWidth = imageWidth;
    int windowHeight = imageHeight + infoLabelHeight + progressHeight + layoutSpacing + layoutMargins;

    // Устанавливаем минимальные размеры
    windowWidth = qMax(windowWidth, 400);
    windowHeight = qMax(windowHeight, 300);

    // Устанавливаем максимальные размеры
    QScreen *primaryScreen = QApplication::primaryScreen();
    if (primaryScreen) {
        QSize screenSize = primaryScreen->availableSize();
        windowWidth = qMin(windowWidth, static_cast<int>(screenSize.width() * 0.9));
        windowHeight = qMin(windowHeight, static_cast<int>(screenSize.height() * 0.9));
    }

    // Меняем размер окна
    //resize(windowWidth, windowHeight);

    // Меняем размер окна и ЗАПРЕЩАЕМ изменение размера
    setFixedSize(windowWidth, windowHeight); // Вместо resize()

    // Запретить изменение ширины, но разрешить высоту:
    //setFixedWidth(windowWidth);
    // или
    // Запретить изменение высоты, но разрешить ширину:
    //setFixedHeight(windowHeight);

    // Центрируем текущую миниатюру после изменения размера
    QTimer::singleShot(50, this, &MainWindow::centerCurrentThumbnail);

    // ЯВНО ЗАДАЕМ ГЕОМЕТРИЮ INFO LABEL
    QRect imageRect = m_imageLabel->geometry();
    int infoY = imageRect.y() + imageRect.height() + 5; // 5px отступ
    m_infoLabel->setGeometry(0, infoY, windowWidth, infoLabelHeight);

    // Обновляем позиции кнопок после изменения размера
    updateButtonPositions();

    // Центрируем окно на экране
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - windowWidth) / 2;
        int y = (screenGeometry.height() - windowHeight) / 2;
        move(x, y);
    }

    qDebug() << "Window resized to:" << windowWidth << "x" << windowHeight;
    qDebug() << "Image:" << imageWidth << "x" << imageHeight;
    qDebug() << "Info label geometry:" << m_infoLabel->geometry();
}

QString MainWindow::getImageSizeText(const QString &imagePath) const
{
    QImage image(imagePath);

    if (image.isNull()) {
        return QString("Ошибка загрузки изображения: %1").arg(QFileInfo(imagePath).fileName());
    }

    // Загружаем текст из соответствующего .txt файла
    QFileInfo imageInfo(imagePath);
    QString textFilePath = imageInfo.absolutePath() + "/" + imageInfo.completeBaseName() + ".txt";
    QString descriptionText = loadTextFromFile(textFilePath);

    if (!descriptionText.isEmpty()) {
        // Если есть текстовый файл, показываем его содержимое
        return descriptionText;
    } else {
        // Если файла с текстом нет, показываем стандартную информацию
        return QString("Шаг %1/%2: %3 (%4 x %5 px)")
            .arg(m_currentIndex + 1)
            .arg(m_imagePaths.size())
            .arg(imageInfo.fileName())
            .arg(image.width())
            .arg(image.height());
    }
}

QString MainWindow::loadTextFromFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "Text file not found:" << filePath;
        return "";
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Читаем файл с автоматическим определением кодировки
        QTextStream in(&file);
        QString text = in.readAll();
        file.close();

        // Убираем лишние переносы строк в начале и конце
        text = text.trimmed();

        qDebug() << "Loaded text from:" << filePath << "Content:" << text.left(50) + "...";
        return text;
    }

    qDebug() << "Failed to open text file:" << filePath;
    return "";
}

void MainWindow::showNotesDialog()
{
    if (m_isWelcomeScreen || m_currentIndex < 0) return;

    NotesDialog dialog(m_currentIndex, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Можно обработать результат если нужно
        qDebug() << "Notes dialog closed";
    }
}

void MainWindow::createProgressIndicator()
{
    // Очищаем предыдущие миниатюры
    for (QLabel *label : m_progressLabels) {
        m_progressLayout->removeWidget(label);
        delete label;
    }
    m_progressLabels.clear();

    // Очищаем layout полностью
    QLayoutItem* item;
    while ((item = m_progressLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_imagePaths.isEmpty()) return;

    // Добавляем растягивающееся пространство слева
    m_progressLayout->addStretch();

    // Создаем миниатюры для каждого изображения
    for (int i = 0; i < m_imagePaths.size(); ++i) {
        QLabel *thumbLabel = new QLabel();
        thumbLabel->setAlignment(Qt::AlignCenter);
        thumbLabel->setFixedSize(50, 40);
        thumbLabel->setStyleSheet("border: 2px solid #cccccc; background-color: #ffffff;");

        // Загружаем миниатюру
        QPixmap original(m_imagePaths[i]);
        if (!original.isNull()) {
            QPixmap thumbnail = original.scaled(40, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            thumbLabel->setPixmap(thumbnail);
        } else {
            thumbLabel->setText(QString::number(i + 1));
            thumbLabel->setStyleSheet("border: 2px solid #cccccc; background-color: #f8f8f8; font-weight: bold;");
        }

        // Выделяем текущее изображение
        if (i == m_currentIndex) {
            thumbLabel->setStyleSheet("border: 3px solid #2196F3; background-color: #e3f2fd;");
        }

        m_progressLayout->addWidget(thumbLabel);
        m_progressLabels.append(thumbLabel);
    }

    // Добавляем растягивающееся пространство справа
    m_progressLayout->addStretch();

    // Центрируем текущее изображение
    centerCurrentThumbnail();
}

void MainWindow::centerCurrentThumbnail()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_progressLabels.size()) return;

    // Принудительно обновляем layout
    m_progressWidget->update();
    m_progressWidget->repaint();
    QApplication::processEvents();

    // Получаем позицию текущей миниатюры
    QLabel *currentThumb = m_progressLabels[m_currentIndex];
    QPoint thumbPos = currentThumb->mapTo(m_progressWidget, QPoint(0, 0));

    // Вычисляем смещение для центрирования
    int thumbCenterX = thumbPos.x() + currentThumb->width() / 2;
    int widgetCenterX = m_progressWidget->width() / 2;
    int offsetX = widgetCenterX - thumbCenterX;

    // Сдвигаем все миниатюры для центрирования текущей
    for (int i = 0; i < m_progressLayout->count(); ++i) {
        QLayoutItem *item = m_progressLayout->itemAt(i);
        if (item && item->widget()) {
            QWidget *widget = item->widget();
            if (QLabel *thumb = qobject_cast<QLabel*>(widget)) {
                thumb->move(thumb->x() + offsetX, thumb->y());
            }
        }
    }
}
