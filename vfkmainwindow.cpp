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
#include "qgsexpression.h"


#include <QSqlDatabase>
#include <QSignalMapper>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <QProgressDialog>
#include <QDesktopServices>


VfkMainWindow::VfkMainWindow( QgisInterface *theQgisInterface, QWidget *parent ) :
  QMainWindow( parent ),
  mQGisIface( theQgisInterface ) ,
  mOgrDataSource( 0 )
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
  connect( vfkBrowser, SIGNAL( showParcely( QStringList ) ), this, SLOT( showParInMap( QStringList ) ) );
  connect( vfkBrowser, SIGNAL( showBudovy( QStringList ) ), this, SLOT( showBudInMap( QStringList ) ) );

  // connect signals from vfkbrowser when changing history
  connect( vfkBrowser, SIGNAL( currentParIdsChanged( bool ) ), selectParButton, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( currentBudIdsChanged( bool ) ), selectBudButton, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( historyBefore( bool ) ), backButton, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( historyAfter( bool ) ), forthButton, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( definitionPointAvailable( bool ) ), cuzkButton, SLOT( setEnabled( bool ) ) );
}

VfkMainWindow::~VfkMainWindow()
{
  if ( mOgrDataSource )
  {
    OGR_DS_Destroy( mOgrDataSource );
    mOgrDataSource = 0;
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
  QString fileName = QFileDialog::getSaveFileName( this, trUtf8( "Jméno exportovaného souboru" ), "",
                                                  tr( "LaTeX (*.tex)") );
  if ( !fileName.isEmpty() )
  {
    vfkBrowser->exportDocument( vfkBrowser->currentUrl(), fileName, VfkTextBrowser::Latex );
  }
}

void VfkMainWindow::on_htmlExportButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, trUtf8( "Jméno exportovaného souboru" ), "",
                                                  tr( "HTML (*.html)" ) );
  if ( !fileName.isEmpty() )
  {
    vfkBrowser->exportDocument( vfkBrowser->currentUrl(), fileName, VfkTextBrowser::Html );
  }
}

void VfkMainWindow::on_selectParButton_clicked()
{
  showInMap( vfkBrowser->currentParIds(), "PAR" );
}

void VfkMainWindow::on_selectBudButton_clicked()
{
  showInMap( vfkBrowser->currentBudIds(), "BUD" );
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

    QFileInfo fileInfo( fileName + ".db" );
    fileInfo.setFile(fileName + "1.db");
    if ( fileInfo.exists()  )
    {
      QMessageBox msgBox;
      msgBox.setText( trUtf8( "Byl nalezen soubor s databází %1. Chcete jej nyní použít?" ).arg( fileInfo.fileName() ));
      msgBox.setInformativeText( trUtf8( "Pokud ne, nalezený soubor bude přepsán." ) );
      msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
      msgBox.setDefaultButton( QMessageBox::Yes );

      int ret = msgBox.exec();
      QString errorMsg;
      switch (ret) {
      case QMessageBox::Yes:
        break;
      case QMessageBox::No:

        if ( !loadVfkFile( fileName, errorMsg ) )
        {
          QString msg2 = trUtf8( "Nepodařilo se získat OGR provider" );
          QMessageBox::critical( this, trUtf8( "Nepodařilo se získat data provider" ), msg2 );

          actionVyhledavani->setEnabled( false );
          return;
        }
        break;
      case QMessageBox::Cancel:
        return;
      default:
        break;
      }
    }
    else
    {
      QString errorMsg;
      if ( !loadVfkFile( fileName, errorMsg ) )
      {
        QString msg2 = trUtf8( "Nepodařilo se získat OGR provider" );
        QMessageBox::critical( this, trUtf8( "Nepodařilo se získat data provider" ), msg2 );

        actionVyhledavani->setEnabled( false );
        return;
      }
    }
    if ( !openDatabase( fileName ) )
    {
      QString msg1 = trUtf8( "Nepodařilo se otevřít databázi." );
      QMessageBox::critical( this, trUtf8( "Chyba" ), msg1 );
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
void VfkMainWindow::showParInMap( QStringList ids )
{
  showInMap( ids, "PAR" );
}

void VfkMainWindow::showBudInMap( QStringList ids )
{
  showInMap( ids, "BUD" );
}

void VfkMainWindow::showInMap( QStringList ids, QString layerName )
{
  if ( mLoadedLayers.contains( layerName )  )
  {
    QString id = mLoadedLayers[ layerName ];
    QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>
        ( QgsMapLayerRegistry::instance()->mapLayer( id ) );
    QString searchString = QString( "ID IN ('%1')" ).arg( ids.join( "','" ) );

    QString error;
    QgsFeatureIds fIds = search( vectorLayer, searchString, error );
    if ( !error.isEmpty() )
    {
      QgsDebugMsg( error );
      return;
    }

    vectorLayer->setSelectedFeatures( fIds );


  }
}

QgsFeatureIds VfkMainWindow::search( QgsVectorLayer *layer, const QString &searchString, QString &error)
{
  // parse search string and build parsed tree
  QgsExpression search( searchString );

  QgsFeatureIds fIds;
  if ( search.hasParserError() )
  {
    error += QObject::tr( "Parsing error:" ) + search.parserErrorString();
    return fIds;
  }
  if ( ! search.prepare( layer->pendingFields() ) )
  {
    error + QObject::tr( "Evaluation error:" ) + search.evalErrorString();
  }

  layer->updateFieldMap();
  layer->select( layer->pendingAllAttributesList(), QgsRectangle(), false);

  QgsFeature f;

  while ( layer->nextFeature( f ) )
  {
    if ( search.evaluate( &f ).toInt() != 0 )
    {
      fIds << f.id();
    }
    // check if there were errors during evaluating
    if ( search.hasEvalError() )
    {
      break;
    }
  }
  return fIds;
}

bool VfkMainWindow::loadVfkFile( const QString &fileName, QString &errorMsg )
{
  if ( mOgrDataSource )
  {
    OGR_DS_Destroy( mOgrDataSource );
    mOgrDataSource = 0;
  }
  QgsApplication::registerOgrDrivers();
  QStringList unusedTables;
//  unusedTables << "ADROBJ" << "ADRUC" << "CABU" <<  "DOCI" << "DOHICI" << "DPM"
//               << "HBPEJ" << "KODCHB" << "NZ" << "NZZP" << "OB" << "OBBP" << "OBDEBO"
//               << "OBESMF" << "OBJRIZ" << "OBPEJ" << "OP" << "POM" << "RECI" << "REZBP"
//               << "RIZKU" << "SBM" << "SBP" << "SOBR" << "SPOL" << "SPOM"
//               << "TYPUCA" << "UCAST" << "UCTYP" << "ZDPAZE" << "ZPMZ" << "ZVB";
  OGRSFDriverH ogrDriver;
  mOgrDataSource = OGROpen( fileName.toUtf8().constData(), false, &ogrDriver );
  if ( !mOgrDataSource )
  {
    errorMsg = tr( "Unable to set open OGR data source" );
    return false;
  }
  else
  {
    QTime t;
    t.start(); // seems to take the same time

    int layerCount = OGR_DS_GetLayerCount( mOgrDataSource );
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
      QString theLayerName = QString::fromUtf8( OGR_FD_GetName( OGR_L_GetLayerDefn( OGR_DS_GetLayer( mOgrDataSource, i ) ) ) );

      if ( unusedTables.contains( theLayerName, Qt::CaseInsensitive ) )
      {
        continue;
      }
      progress.setLabelText( tr( "Loading layer %1: %2" ).arg( i ).arg( theLayerName ) );

      OGR_L_GetFeatureCount( OGR_DS_GetLayer( mOgrDataSource, i ), 1 ) ;
    }

    qDebug("\nTime elapsed: %d ms\n", t.elapsed());
  }
  return true;
}

//bool VfkMainWindow::setDataProvider( QString dataSource )
//{
//  mDataProvider =
//      ( QgsVectorDataProvider* )( QgsProviderRegistry::instance()->provider( "ogr", dataSource ) );

//  if ( mDataProvider )
//  {
//    QgsDebugMsg( "Instantiated the data provider plugin" );

//    bool valid = mDataProvider->isValid();
//    if ( !valid )
//    {
//      QgsDebugMsg( "Invalid provider plugin " + QString( dataSource.toUtf8() ) );
//      return false;
//    }
//  }
//  else
//  {
//    QgsDebugMsg( "Unable to get data provider" );
//    return false;
//  }

//  return true;

//}

void VfkMainWindow::loadVfkLayer( QString vfkLayerName )
{
  QgsDebugMsg( QString( "Loading vfk layer %1" ).arg( vfkLayerName ) );
  if ( mLoadedLayers.contains( vfkLayerName ) )
  {
    QgsDebugMsg( QString( "Vfk layer %1 is already loaded" ).arg( vfkLayerName ) );
    return;
  }
  QString composedURI = mLastVfkFile + "|layername=" + vfkLayerName;
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


void VfkMainWindow::on_cuzkButton_clicked()
{
  QString x = vfkBrowser->currentDefinitionPoint().first.split( "." ).at( 0 );
  QString y = vfkBrowser->currentDefinitionPoint().second.split( "." ).at( 0 );
  QString url = QString( "http://nahlizenidokn.cuzk.cz/MapaIdentifikace.aspx?&x=-%1&y=-%2" ).arg( y ).arg( x );
  QDesktopServices::openUrl( QUrl( url, QUrl::TolerantMode) );
}
