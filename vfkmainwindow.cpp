#include "vfkmainwindow.h"
#include "ui_vfkmainwindow.h"
#include "searchformcontroller.h"
#include "vfktextbrowser.h"

#include "qgsvectordataprovider.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsapplication.h"

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>

#include <QSqlDatabase>
#include <QSignalMapper>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <QProgressDialog>


VfkMainWindow::VfkMainWindow( QgisInterface *theQgisInterface, QWidget *parent ) :
  QMainWindow( parent ),
  mQGisIface( theQgisInterface ) ,
  mDataProvider( 0 )
{

  setupUi( this );

  QActionGroup *actionGroup = new QActionGroup( this );
  actionGroup->addAction( actionImport );
  actionGroup->addAction( actionVyhledavani );

  QSignalMapper* signalMapper = new QSignalMapper( this );

  // connect to `clicked' on all buttons
  connect( actionImport, SIGNAL( triggered() ), signalMapper, SLOT( map() ) );
  connect( actionVyhledavani, SIGNAL( triggered() ), signalMapper, SLOT( map() ) );

  // setMapping on each button to the QStackedWidget index we'd like to switch to
  // note: this affects the value passed via QSignalMapper::mapped(int) signal
  signalMapper->setMapping( actionImport, 0);
  signalMapper->setMapping( actionVyhledavani, 1);

  // finally, connect the mapper to the stacked widget
  connect( signalMapper, SIGNAL( mapped( int ) ), stackedWidget, SLOT( setCurrentIndex( int ) ) );
  actionImport->trigger();
  actionVyhledavani->setEnabled( false );

  mDefaultPalette = vfkFileLineEdit->palette();


  SearchFormController::MainControls searchFormMainControls;
  searchFormMainControls.formComboBox = searchCombo;
  searchFormMainControls.searchForms = searchForms;
  searchFormMainControls.searchButton = searchButton;

  SearchFormController::SearchForms searchForms;
  searchForms.vlastnici = vlastniciSearchForm;
  searchForms.parcely = parcelySearchForm;
  searchForms.budovy = budovySearchForm;
  searchForms.jednotky = jednotkySearchForm;

  mSearchController = new SearchFormController(searchFormMainControls, searchForms, this);

  connect( mSearchController, SIGNAL( actionTriggered( QUrl ) ), vfkBrowser, SLOT( processAction( QUrl ) ) );
}

VfkMainWindow::~VfkMainWindow()
{
  if ( mDataProvider )
  {
    delete mDataProvider;
  }
  QSqlDatabase::database( property("connectionName").toString() ).close();
  QSqlDatabase::removeDatabase( property("connectionName").toString() );
}

bool VfkMainWindow::openDatabase( QString dbPath )
{
  QString connectionName = QUuid::createUuid().toString();
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", connectionName );
  dbPath += ".db";
  QgsDebugMsg( dbPath );
  db.setDatabaseName( dbPath );
  bool ok = db.open();
  if ( !ok )
  {
    return false;
  }
  setProperty( "connectionName", connectionName );
  return true;
}

void VfkMainWindow::on_backButton_clicked()
{
  vfkBrowser->goBack();
}

void VfkMainWindow::on_forthButton_clicked()
{
  vfkBrowser->goForth();
}


void VfkMainWindow::on_latexExportButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(this, trUtf8("Jméno exportovaného souboru"), "",
                                                  tr("(La)TeX (*.tex)"));
  if ( !fileName.isEmpty() )
  {
    vfkBrowser->exportDocument( vfkBrowser->currentUrl(), fileName, VfkTextBrowser::Latex );
  }
}

void VfkMainWindow::on_browseButton_clicked()
{
  QString title = trUtf8( "Otevřít VFK soubor" );
  QSettings settings;
  QString lastUsedDir = settings.value( QString("/UI/") + "lastVectorFileFilter" + "Dir", "." ).toString();
  QString fileName = QFileDialog::getOpenFileName( this, title, lastUsedDir, "VFK (*.vfk)" );
  if ( fileName.isEmpty() )
  {
    return;
  }
  vfkFileLineEdit->setText( fileName );

}

void VfkMainWindow::on_loadVfkButton_clicked()
{
  QString fileName = vfkFileLineEdit->text();

  if ( mLastVfkFile != fileName )
  {
    QString errorMsg;
    if ( !loadVfkFile( fileName, errorMsg ) )
    {
      QString msg = trUtf8( "Nepodařilo se získat OGR provider" );
      QMessageBox::critical( this, trUtf8( "Nepodařilo se získat data provider" ), msg );

      actionVyhledavani->setEnabled( false );
      return;
    }

    vfkBrowser->setConnectionName( property( "connectionName" ).toString() );
    mSearchController->setConnectionName( property( "connectionName" ).toString() );
    actionVyhledavani->setEnabled( true );

    mLastVfkFile = fileName;

    mLoadedLayers.clear();
  }

  if ( parCheckBox->isChecked() )
  {
    loadVfkLayer( "PAR" );
  }
  else
  {
    unLoadVfkLayer( "PAR" );
  }

  if ( budCheckBox->isChecked() )
  {
    loadVfkLayer( "BUD" );
  }
  else
  {
    unLoadVfkLayer( "BUD" );
  }


}


void VfkMainWindow::on_vfkFileLineEdit_textChanged( const QString &arg1 )
{
  QFileInfo info( arg1 );
  if ( info.isFile() )
  {
    loadVfkButton->setEnabled( true );

    vfkFileLineEdit->setPalette( mDefaultPalette );
  }
  else
  {
    loadVfkButton->setEnabled( false );

    QPalette pal = vfkFileLineEdit->palette();
    pal.setColor( QPalette::Text, Qt::red );
    vfkFileLineEdit->setPalette( pal );
  }
}

bool VfkMainWindow::loadVfkFile( const QString &fileName, QString &errorMsg )
{
  mDataSource = fileName;
  if ( mDataProvider )
  {
    delete mDataProvider;
    mDataProvider = 0;
  }
//  bool success = setDataProvider( mDataSource );
  OGRSFDriverH ogrDriver;
  OGRDataSourceH dataSource = OGROpen( fileName.toUtf8().constData(), false, &ogrDriver );
  bool success = true;
  if ( !success )
  {
    errorMsg = tr( "Unable to set data provider" );
    return false;
  }
  else
  {
    QTime t;
    t.start(); // seems to take the same time

    QgsApplication::registerOgrDrivers();

    int layerCount = OGR_DS_GetLayerCount( dataSource );
    QProgressDialog progress( this );
    progress.setLabelText( tr( "Loading layers" ) );
    progress.setRange( 0, layerCount );
    progress.setModal( true );
    progress.show();
    progress.setValue( 0 );


    for ( int i = 0; i < layerCount; i++ )
    {
      qApp->processEvents();
      if ( progress.wasCanceled() )
      {
        errorMsg = tr( "Opening database stopped" );
        return false;
      }
      progress.setValue(i);
      QString theLayerName = QString::fromUtf8( OGR_FD_GetName( OGR_L_GetLayerDefn( OGR_DS_GetLayer( dataSource, i ) ) ) );
      progress.setLabelText( tr( "Loading layer %1: %2" ).arg( i ).arg( theLayerName ) );

      OGR_L_GetFeatureCount( OGR_DS_GetLayer( dataSource, i ), 1 ) ;
    }

//    mDataProvider->subLayers();

    qDebug("\nTime elapsed: %d ms\n", t.elapsed());
//    QString dbPath = mDataProvider->dataSourceUri();
    QString dbPath=fileName;
//    dbPath.truncate( dbPath.lastIndexOf( "|" ) );
    if ( !openDatabase( dbPath ) )
    {
      errorMsg = tr( "Failed to open database %1" ).arg( dbPath );
      return false;
    }
  }
  return true;
}

bool VfkMainWindow::setDataProvider( QString dataSource )
{
  mDataProvider =
      ( QgsVectorDataProvider* )( QgsProviderRegistry::instance()->provider( "ogr", dataSource ) );

  if ( mDataProvider )
  {
    QgsDebugMsg( "Instantiated the data provider plugin" );

    bool valid = mDataProvider->isValid();
    if ( !valid )
    {
      QgsDebugMsg( "Invalid provider plugin " + QString( dataSource.toUtf8() ) );
      return false;
    }
  }
  else
  {
    QgsDebugMsg( "Unable to get data provider" );
    return false;
  }

  return true;

}

void VfkMainWindow::loadVfkLayer( QString vfkLayerName )
{
  QgsDebugMsg( QString( "Loading vfk layer %1" ).arg( vfkLayerName ) );
  if ( mLoadedLayers.contains( vfkLayerName ) )
  {
    QgsDebugMsg( QString( "Vfk layer %1 is already loaded" ).arg( vfkLayerName ) );
    return;
  }
  QString composedURI = mDataSource + "|layername=" + vfkLayerName;
  QgsVectorLayer *layer = mQGisIface->addVectorLayer( composedURI, vfkLayerName, "ogr" );
  mLoadedLayers.insert( vfkLayerName, layer->id() );
}

void VfkMainWindow::unLoadVfkLayer( QString vfkLayerName )
{
  QgsDebugMsg( QString( "Unloading vfk layer %1" ).arg( vfkLayerName ) );
  if ( !mLoadedLayers.contains( vfkLayerName ) )
  {
    QgsDebugMsg( QString( "Vfk layer %1 is already unloaded" ).arg( vfkLayerName ) );
    return;
  }
  QgsMapLayerRegistry::instance()->removeMapLayer( mLoadedLayers.value( vfkLayerName ), true );
  mLoadedLayers.remove( vfkLayerName );
}


