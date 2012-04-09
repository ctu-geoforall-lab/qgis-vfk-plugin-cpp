#ifndef VFKMAINWINDOW_H
#define VFKMAINWINDOW_H

#include "documentbuilder.h"
#include "ui_vfkmainwindow.h"

#include "qgsdataprovider.h"
#include "qgsvectorlayer.h"
#include <qgisinterface.h>
#include <ogr_api.h>

#include <QMainWindow>
#include <QUrl>
#include <QToolButton>

class SearchFormController;

typedef QMap<QString, QString> LayerNameIdMap;

class VfkMainWindow : public QMainWindow, private Ui::VfkMainWindow
{
    Q_OBJECT

  enum VfkLayer { Par, Bud };

public:
    explicit VfkMainWindow(QgisInterface *theQgisInterface, QWidget *parent = 0);
  ~VfkMainWindow();

  bool loadVfkFile( const QString &fileName, QString &errorMsg );

public slots:

signals:
  void goBack();
  void searchOpsubByName( QString );
  void refreshLegend( QgsMapLayer *layer );

private slots:
    void browserGoBack();

    void browserGoForward();

    void latexExport();
    void htmlExport();
    void on_browseButton_clicked();
    void on_loadVfkButton_clicked();
    void selectParInMap();
    void selectBudInMap();

    void on_vfkFileLineEdit_textChanged(const QString &arg1);
    void showParInMap( QStringList ids );
    void showBudInMap( QStringList ids);
    void showInMap( QStringList ids, QString layerName );
    void showOnCuzk();
    void showInfoAboutSelection();
    void setSelectionChangedConnected( bool connected );
    void switchToImport();
    void switchToSearch( int searchType);

private:
    QgisInterface *mQGisIface;
    QString mLastVfkFile;
    QPalette mDefaultPalette;
    OGRDataSourceH mOgrDataSource;
//    QString mDataSourceName;
    LayerNameIdMap mLoadedLayers;

    QToolBar *mBrowserToolbar;
    QToolBar *mainToolBar;

    void createToolbarsAndConnect();
    bool openDatabase(QString dbPath);
//    bool setDataProvider( QString dataSource );
    void loadVfkLayer( QString vfkLayerName );
    void unLoadVfkLayer( QString vfkLayerName );
    QgsFeatureIds search( QgsVectorLayer *layer, const QString &searchString, QString &error);
    QStringList selectedIds( QgsVectorLayer *layer );
    bool setSymbology( QgsVectorLayer *layer );


    SearchFormController *mSearchController;
};

#endif // VFKMAINWINDOW_H
