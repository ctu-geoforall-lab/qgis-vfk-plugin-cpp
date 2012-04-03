#ifndef VFKMAINWINDOW_H
#define VFKMAINWINDOW_H

#include "documentbuilder.h"
#include "ui_vfkmainwindow.h"

#include "qgsdataprovider.h"
#include <qgisinterface.h>

#include <QMainWindow>
#include <QUrl>

class SearchFormController;

typedef QMap<QString, QString> LayerNameIdMap;

class VfkMainWindow : public QMainWindow, private Ui::VfkMainWindow
{
    Q_OBJECT

public:
    explicit VfkMainWindow(QgisInterface *theQgisInterface, QWidget *parent = 0);
  ~VfkMainWindow();

  bool loadVfkFile( const QString &fileName, QString &errorMsg );

public slots:

signals:
  void goBack();
  void searchOpsubByName( QString );

private slots:
    void on_backButton_clicked();

    void on_forthButton_clicked();

    void on_latexExportButton_clicked();
    void on_browseButton_clicked();
    void on_loadVfkButton_clicked();

    void on_vfkFileLineEdit_textChanged(const QString &arg1);

private:
    QgisInterface *mQGisIface;
    QString mLastVfkFile;
    QPalette mDefaultPalette;
    QString mDataSource;
    QgsDataProvider *mDataProvider;
    LayerNameIdMap mLoadedLayers;

    bool openDatabase(QString dbPath);
    bool setDataProvider( QString dataSource );
    void loadVfkLayer( QString vfkLayerName );
    void unLoadVfkLayer( QString vfkLayerName );

    SearchFormController *mSearchController;
};

#endif // VFKMAINWINDOW_H
