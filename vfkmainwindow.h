#ifndef VFKMAINWINDOW_H
#define VFKMAINWINDOW_H

#include "documentbuilder.h"
#include "ui_vfkmainwindow.h"

#include <QMainWindow>
#include <QUrl>

class SearchFormController;

class VfkMainWindow : public QMainWindow, private Ui::VfkMainWindow
{
    Q_OBJECT

public:
    explicit VfkMainWindow(QWidget *parent = 0);
    ~VfkMainWindow();

public slots:

signals:
  void goBack();
  void searchOpsubByName( QString );

private slots:
    void on_backButton_clicked();

    void on_forthButton_clicked();

    void on_searchButton_clicked();
    void on_latexExportButton_clicked();

private:
    bool openDatabase();

    SearchFormController *mSearchController;
};

#endif // VFKMAINWINDOW_H
