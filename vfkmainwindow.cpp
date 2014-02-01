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
#include "qgslegendinterface.h"
#include "qgsmapcanvas.h"


#include <QSqlDatabase>
#include <QSignalMapper>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QMenu>
#include <QToolBar>


VfkMainWindow::VfkMainWindow( QgisInterface *theQgisInterface, QWidget *parent ) :
  QMainWindow( parent ),
  mQGisIface( theQgisInterface ) ,
  mOgrDataSource( 0 )
{

  setupUi( this );
  createToolbarsAndConnect();

  loadVfkButton->setDisabled( true );

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
  connect( this, SIGNAL( enableSearch( bool ) ), searchButton, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( showParcely( QStringList ) ), this, SLOT( showParInMap( QStringList ) ) );
  connect( vfkBrowser, SIGNAL( showBudovy( QStringList ) ), this, SLOT( showBudInMap( QStringList ) ) );

  QgsLegendInterface *legendIface = mQGisIface->legendInterface();
  connect( this, SIGNAL ( refreshLegend( QgsMapLayer* ) ), legendIface, SLOT( refreshLayerSymbology( QgsMapLayer* ) ) );

  // needed because of scale dependent labels
  QgsMapRenderer* myRenderer = mQGisIface->mapCanvas()->mapRenderer();
  myRenderer->setMapUnits( QGis::Meters );

  vfkBrowser->showHelpPage();
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

void VfkMainWindow::browserGoBack()
{
  vfkBrowser->goBack();
}

void VfkMainWindow::browserGoForward()
{
  vfkBrowser->goForth();
}


void VfkMainWindow::latexExport()
{
  QString fileName = QFileDialog::getSaveFileName( this, trUtf8( "Jméno exportovaného souboru" ), "",
                                                  tr( "LaTeX (*.tex)") );
  if ( !fileName.isEmpty() )
  {
    vfkBrowser->exportDocument( vfkBrowser->currentUrl(), fileName, VfkTextBrowser::Latex );
  }
}

void VfkMainWindow::htmlExport()
{
  QString fileName = QFileDialog::getSaveFileName( this, trUtf8( "Jméno exportovaného souboru" ), "",
                                                  tr( "HTML (*.html)" ) );
  if ( !fileName.isEmpty() )
  {
    vfkBrowser->exportDocument( vfkBrowser->currentUrl(), fileName, VfkTextBrowser::Html );
  }
}

void VfkMainWindow::selectParInMap()
{
  showInMap( vfkBrowser->currentParIds(), "PAR" );
}

void VfkMainWindow::selectBudInMap()
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
    QString errorMsg;
    if ( !loadVfkFile( fileName, errorMsg ) )
    {
      QString msg2 = trUtf8( "Nepodařilo se získat OGR provider" );
      QMessageBox::critical( this, trUtf8( "Nepodařilo se získat data provider" ), msg2 );

      emit enableSearch( false );
      return;
    }

    if ( !openDatabase( fileName ) )
    {
      QString msg1 = trUtf8( "Nepodařilo se otevřít databázi." );
      QMessageBox::critical( this, trUtf8( "Chyba" ), msg1 );
      emit enableSearch( false );
      return;
    }
    vfkBrowser->setConnectionName( property( "connectionName" ).toString() );
    mSearchController->setConnectionName( property( "connectionName" ).toString() );
    emit enableSearch( true );

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
  if ( actionShowInfoaboutSelection->isChecked() )
  {
    setSelectionChangedConnected( false );
    showInMap( ids, "PAR" );
    setSelectionChangedConnected( true );
  }
  else
  {
    showInMap( ids, "PAR" );
  }
}

void VfkMainWindow::showBudInMap( QStringList ids )
{
  if ( actionShowInfoaboutSelection->isChecked() )
  {
    setSelectionChangedConnected( false );
    showInMap( ids, "BUD" );
    setSelectionChangedConnected( true );
  }
  else
  {
    showInMap( ids, "BUD" );
  }
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
  QgsRectangle rect;
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

  layer->select( rect, false);

  QgsFeatureIterator fit = layer->getFeatures();
  QgsFeature f;

  while ( fit.nextFeature( f ) )
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

  QProgressDialog progress( this );
  progress.setWindowTitle( trUtf8 ("Načítám VFK data...") );
  progress.setLabelText( trUtf8( "Načítám data do SQLite databáze (může nějaký čas trvat...)" ) );
  progress.setModal( true );
  progress.show();
  qApp->processEvents(); // force to show dialog before opening OGR datasource

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
    QString extraMsg;

    t.start(); // seems to take the same time

    QgsDebugMsg( QString( "Creating new DB: %1" ).arg( !OGR_DS_TestCapability( mOgrDataSource, "IsPreProcessed" ) ) );

    int layerCount = OGR_DS_GetLayerCount( mOgrDataSource );

    if ( !OGR_DS_TestCapability( mOgrDataSource, "IsPreProcessed" ) )
        extraMsg = trUtf8( "Načítám data do SQLite databáze (může nějaký čas trvat...)" );

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
      progress.setLabelText( trUtf8( "VFK data %1/%2: %3\n%4" ).arg( i ).arg( layerCount ).arg( theLayerName ).arg( extraMsg ) );

      OGR_L_GetFeatureCount( OGR_DS_GetLayer( mOgrDataSource, i ), 1 ) ;
    }

    QgsDebugMsg( QString( "TIME ELAPSED: %1 ms" ).arg( t.elapsed() ) );
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
  QgsVectorLayer *layer = new QgsVectorLayer( composedURI, vfkLayerName, "ogr" );
  mLoadedLayers.insert( vfkLayerName, layer->id() );
  setSymbology( layer );

  QList<QgsMapLayer *> myList;
  myList << layer;
  QgsMapLayerRegistry::instance()->addMapLayers( myList );
}

void VfkMainWindow::unLoadVfkLayer( QString vfkLayerName )
{
  QgsDebugMsg( QString( "Unloading vfk layer %1" ).arg( vfkLayerName ) );
  if ( !mLoadedLayers.contains( vfkLayerName ) )
  {
    QgsDebugMsg( QString( "Vfk layer %1 is already unloaded" ).arg( vfkLayerName ) );
    return;
  }
  QgsMapLayerRegistry::instance()->removeMapLayers( QStringList() << mLoadedLayers.value( vfkLayerName ));
  mLoadedLayers.remove( vfkLayerName );
}

bool VfkMainWindow::setSymbology( QgsVectorLayer *layer )
{
  QString name = layer->name();
  QString symbologyFile;

  // which style file has to be used
  if ( name == "PAR" )
  {
    symbologyFile = ":/vfkplugin/parStyle.qml";
  }
  else if ( name == "BUD" )
  {
    symbologyFile = ":/vfkplugin/budStyle.qml";
  }
  bool resultFlag;
  QString errorMsg = layer->loadNamedStyle( symbologyFile, resultFlag );
  if ( !resultFlag )
  {
    QMessageBox::information( this, tr( "Load Style" ), errorMsg );
  }

  layer->triggerRepaint();

  emit refreshLegend( layer );

  return true;
}

void VfkMainWindow::showOnCuzk()
{
  QString x = vfkBrowser->currentDefinitionPoint().first.split( "." ).at( 0 );
  QString y = vfkBrowser->currentDefinitionPoint().second.split( "." ).at( 0 );
  QString url = QString( "http://nahlizenidokn.cuzk.cz/MapaIdentifikace.aspx?&x=-%1&y=-%2" ).arg( y ).arg( x );
  QDesktopServices::openUrl( QUrl( url, QUrl::TolerantMode) );
}

void VfkMainWindow::showInfoAboutSelection()
{
  QStringList layers = QStringList() << "PAR" << "BUD";
  QMap<QString, QStringList> layerIds;
  foreach( QString layer, layers )
  {
    if ( mLoadedLayers.contains( layer )  )
    {
      QString id = mLoadedLayers[ layer ];
      QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>
          ( QgsMapLayerRegistry::instance()->mapLayer( id ) );
      layerIds[ layer ] = selectedIds( vectorLayer );
    }
  }
  vfkBrowser->showInfoAboutSelection( layerIds[ "PAR" ], layerIds[ "BUD" ] );
}

void VfkMainWindow::setSelectionChangedConnected( bool connected )
{
  for ( LayerNameIdMap::const_iterator it = mLoadedLayers.constBegin(); it != mLoadedLayers.constEnd(); ++it )
  {
    QString id = it.value();
    QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>
        ( QgsMapLayerRegistry::instance()->mapLayer( id ) );
    if ( connected )
    {
        connect( vectorLayer, SIGNAL( selectionChanged() ), this, SLOT( showInfoAboutSelection() ) );
    }
    else
    {
      disconnect( vectorLayer, SIGNAL( selectionChanged() ), this, SLOT( showInfoAboutSelection() ) );
    }
  }

}

void VfkMainWindow::switchToImport()
{
  actionImport->trigger();
}

void VfkMainWindow::switchToSearch( int searchType )
{
  actionVyhledavani->trigger();
  searchCombo->setCurrentIndex( searchType );
  searchForms->setCurrentIndex( searchType );

}

QStringList VfkMainWindow::selectedIds( QgsVectorLayer *layer ) // should be const
{
  QStringList ids;
  QgsFeatureList flist = layer->selectedFeatures();

  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    QgsFeature f = *it;
    ids << f.attribute( "ID" ).toString();
  }
  return ids;
}

void VfkMainWindow::createToolbarsAndConnect()
{
  // main toolbar
  mainToolBar = new QToolBar( this );
  mainToolBar->addAction( actionImport );
  mainToolBar->addAction( actionVyhledavani );
  mainToolBar->setOrientation( Qt::Vertical );
  addToolBar( Qt::LeftToolBarArea, mainToolBar );

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
//  actionVyhledavani->setEnabled( false );

  connect( vfkBrowser, SIGNAL( switchToPanelImport() ), this, SLOT( switchToImport() ) );
  connect( vfkBrowser, SIGNAL( switchToPanelSearch( int ) ), this, SLOT( switchToSearch( int ) ) );


  // browser toolbar
  mBrowserToolbar = new QToolBar( this );
  connect( actionBack, SIGNAL( triggered() ), vfkBrowser, SLOT( goBack() ) );
  connect( actionForward, SIGNAL( triggered() ), vfkBrowser, SLOT( goForth() ) );
  connect( actionSelectParInMap, SIGNAL( triggered() ), this, SLOT( selectParInMap() ) );
  connect( actionSelectBudInMap, SIGNAL( triggered() ), this, SLOT( selectBudInMap() ) );
  connect( actionCuzkPage, SIGNAL( triggered() ), this, SLOT( showOnCuzk() ) );
  connect( actionExportLatex, SIGNAL( triggered() ), this, SLOT( latexExport() ) );
  connect( actionExportHtml, SIGNAL( triggered() ), this, SLOT( htmlExport() ) );
  connect( actionShowInfoaboutSelection, SIGNAL( toggled( bool ) ), this, SLOT( setSelectionChangedConnected( bool ) ) );
  connect( actionShowHelpPage, SIGNAL( triggered() ), vfkBrowser, SLOT( showHelpPage() ) );

  QToolButton *bt = new QToolButton( mBrowserToolbar );
  bt->setPopupMode( QToolButton::InstantPopup );
  bt->setText( tr( "Export" ) );
  QMenu *menu = new QMenu( bt );
  menu->addAction( actionExportLatex );
  menu->addAction( actionExportHtml );
  bt->setMenu( menu );

  mBrowserToolbar->addAction( actionBack );
  mBrowserToolbar->addAction( actionForward );
  mBrowserToolbar->addAction( actionSelectParInMap );
  mBrowserToolbar->addAction( actionSelectBudInMap );
  mBrowserToolbar->addAction( actionCuzkPage );
  mBrowserToolbar->addSeparator();
  mBrowserToolbar->addAction( actionShowInfoaboutSelection );
  mBrowserToolbar->addSeparator();
  mBrowserToolbar->addWidget( bt );
  mBrowserToolbar->addSeparator();
  mBrowserToolbar->addAction( actionShowHelpPage );

  rightWidgetLayout->insertWidget( 0, mBrowserToolbar );

  // connect signals from vfkbrowser when changing history
  connect( vfkBrowser, SIGNAL( currentParIdsChanged( bool ) ), actionSelectParInMap, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( currentBudIdsChanged( bool ) ), actionSelectBudInMap, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( historyBefore( bool ) ), actionBack, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( historyAfter( bool ) ), actionForward, SLOT( setEnabled( bool ) ) );
  connect( vfkBrowser, SIGNAL( definitionPointAvailable( bool ) ), actionCuzkPage, SLOT( setEnabled( bool ) ) );
}
