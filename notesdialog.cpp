#include "notesdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

NotesDialog::NotesDialog(int currentStep, QWidget *parent)
    : QDialog(parent)
    , m_currentStep(currentStep)
{
    setWindowTitle("Замечания - Шаг " + QString::number(currentStep + 1));
    setMinimumSize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Список замечаний
    m_notesList = new QListWidget(this);
    mainLayout->addWidget(m_notesList);

    // Поле редактирования
    m_noteEdit = new QTextEdit(this);
    m_noteEdit->setPlaceholderText("Введите ваше замечание...");
    m_noteEdit->setMaximumHeight(100);
    mainLayout->addWidget(m_noteEdit);

    // Кнопки
    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    m_addButton = new QPushButton("Добавить", this);
    m_editButton = new QPushButton("Редактировать", this);
    m_deleteButton = new QPushButton("Удалить", this);

    buttonsLayout->addWidget(m_addButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_editButton);
    buttonsLayout->addWidget(m_deleteButton);

    mainLayout->addLayout(buttonsLayout);

    // Загружаем замечания
    loadNotes();
    updateButtons();

    // Подключаем сигналы
    connect(m_addButton, &QPushButton::clicked, this, &NotesDialog::addNote);
    connect(m_editButton, &QPushButton::clicked, this, &NotesDialog::editNote);
    connect(m_deleteButton, &QPushButton::clicked, this, &NotesDialog::deleteNote);
    connect(m_notesList, &QListWidget::currentRowChanged, this, &NotesDialog::updateButtons);
}

NotesDialog::~NotesDialog()
{
    saveNotes();
}

void NotesDialog::loadNotes()
{
    // Создаем папку resources если ее нет
    QDir resourcesDir("resources");
    if (!resourcesDir.exists()) {
        resourcesDir.mkpath(".");
    }

    QString fileName = QString("resources/notes_step%1.txt").arg(m_currentStep + 1);
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (!line.isEmpty()) {
                m_notesList->addItem(line);
            }
        }
        file.close();
    }
}

void NotesDialog::saveNotes()
{
    // Создаем папку resources если ее нет
    QDir resourcesDir("resources");
    if (!resourcesDir.exists()) {
        resourcesDir.mkpath(".");
    }

    QString fileName = QString("resources/notes_step%1.txt").arg(m_currentStep + 1);
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < m_notesList->count(); ++i) {
            out << m_notesList->item(i)->text() << "\n";
        }
        file.close();
    }
}

QString NotesDialog::getCurrentDateTime() const
{
    return QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm");
}

void NotesDialog::addNote()
{
    QString noteText = m_noteEdit->toPlainText().trimmed();
    if (noteText.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Замечание не может быть пустым!");
        return;
    }

    QString noteWithTime = QString("[%1] %2").arg(getCurrentDateTime()).arg(noteText);
    m_notesList->addItem(noteWithTime);
    m_noteEdit->clear();

    saveNotes();
}

void NotesDialog::editNote()
{
    int currentRow = m_notesList->currentRow();
    if (currentRow < 0) return;

    QString currentText = m_notesList->currentItem()->text();
    // Извлекаем только текст без времени
    QString noteText = currentText.mid(currentText.indexOf("]") + 2);

    // Создаем диалог для редактирования
    QDialog editDialog(this);
    editDialog.setWindowTitle("Редактирование замечания");
    editDialog.setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&editDialog);

    QTextEdit *textEdit = new QTextEdit(&editDialog);
    textEdit->setPlainText(noteText);
    textEdit->setMinimumHeight(200);
    layout->addWidget(textEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("Сохранить", &editDialog);
    QPushButton *cancelButton = new QPushButton("Отмена", &editDialog);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &editDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &editDialog, &QDialog::reject);

    if (editDialog.exec() == QDialog::Accepted) {
        QString newText = textEdit->toPlainText().trimmed();
        if (!newText.isEmpty()) {
            QString newNoteWithTime = QString("[%1] %2").arg(getCurrentDateTime()).arg(newText);
            m_notesList->currentItem()->setText(newNoteWithTime);
            saveNotes();
        }
    }
}

void NotesDialog::deleteNote()
{
    int currentRow = m_notesList->currentRow();
    if (currentRow < 0) return;

    if (QMessageBox::question(this, "Подтверждение",
                              "Удалить выбранное замечание?") == QMessageBox::Yes) {
        delete m_notesList->takeItem(currentRow);
        saveNotes();
        updateButtons();
    }
}

void NotesDialog::updateButtons()
{
    bool hasSelection = m_notesList->currentRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

QString NotesDialog::getNotes() const
{
    QStringList notes;
    for (int i = 0; i < m_notesList->count(); ++i) {
        notes << m_notesList->item(i)->text();
    }
    return notes.join("\n");
}

void NotesDialog::setNotes(const QString &notes)
{
    m_notesList->clear();
    QStringList noteList = notes.split("\n", Qt::SkipEmptyParts);
    foreach (const QString &note, noteList) {
        if (!note.trimmed().isEmpty()) {
            m_notesList->addItem(note);
        }
    }
}
