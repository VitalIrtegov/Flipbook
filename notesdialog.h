#ifndef NOTESDIALOG_H
#define NOTESDIALOG_H

#include <QDialog>
#include <QObject>
#include <QListWidget>

class QPushButton;
class QTextEdit;

class NotesDialog : public QDialog {
    Q_OBJECT

public:
    explicit NotesDialog(int currentStep, QWidget *parent = nullptr);
    ~NotesDialog();

    QString getNotes() const;
    void setNotes(const QString &notes);

private slots:
    void addNote();
    void editNote();
    void deleteNote();
    void updateButtons();

private:
    void loadNotes();
    void saveNotes();
    QString getCurrentDateTime() const;

    QListWidget *m_notesList;
    QTextEdit *m_noteEdit;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    int m_currentStep;
};

#endif // NOTESDIALOG_H
