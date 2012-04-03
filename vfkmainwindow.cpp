#include "vfkmainwindow.h"
#include "ui_vfkmainwindow.h"

#include "searchformcontroller.h"
#include "vfktextbrowser.h"

#include <QSqlDatabase>
#include <QFileDialog>
#include <QDebug>


VfkMainWindow::VfkMainWindow(QWidget *parent) :
  QMainWindow(parent)
{
  if ( !openDatabase() )
  {
    qDebug() << "failed to open Database";
  }
  setupUi( this );
  //  connect( ui->backButton, SIGNAL( clicked() ), ui->vfkBrowser, SLOT( backward()) );
  //  connect( ui->forthButton, SIGNAL( clicked() ), ui->vfkBrowser, SLOT( forward()) );
  //  connect( this, SIGNAL( searchOpsubByName( QString ) ), ui->vfkBrowser, SLOT( searchOpsubByName( QString ) ) );

  vfkBrowser->startPage();

  SearchFormController::MainControls searchFormMainControls;
  searchFormMainControls.formComboBox = searchCombo;
  searchFormMainControls.searchForms = searchForms;
  searchFormMainControls.searchButton = searchButton;

  SearchFormController::SearchForms searchForms;
  searchForms.vlastnici = vlastniciSearchForm;
  searchForms.parcely = parcelySearchForm;
  searchForms.budovy = budovySearchForm;
  searchForms.jednotky = jednotkySearchForm;

  mSearchController = new SearchFormController(searchFormMainControls, searchForms,
                                               property("connectionName").toString(), this);

  connect( mSearchController, SIGNAL( actionTriggered( QUrl ) ), vfkBrowser, SLOT( processAction( QUrl ) ) );
}

VfkMainWindow::~VfkMainWindow()
{
  QSqlDatabase::database( property("connectionName").toString() ).close();
  QSqlDatabase::removeDatabase( property("connectionName").toString() );
}

bool VfkMainWindow::openDatabase()
{
  QString connectionName = "vfkConnection";
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", connectionName );
  QString dbPath = "../../example_files/data1.vfk.db";
  db.setDatabaseName( dbPath );
  bool ok = db.open();
  if ( !ok )
  {
    qWarning() << "expecting database in" << dbPath;
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

void VfkMainWindow::on_searchButton_clicked()
{
  //  emit searchOpsubByName( ui->searchEdit->text() );
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

