#include "searchformcontroller.h"

#include "vlastnicisearchform.h"
#include "parcelysearchform.h"
#include "budovysearchform.h"
#include "jednotkysearchform.h"

#include <QDebug>
#include <QUrl>
#include <QRegExp>
#include <QStandardItemModel>

#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>

SearchFormController::SearchFormController(const MainControls &mainControls, const SearchForms &searchForms, QObject *parent)
  : QObject( parent ), controls( mainControls ), forms( searchForms )
{

  controls.formComboBox->addItem( QObject::trUtf8( "vlastníci" ), Vlastnici );
  controls.formComboBox->addItem( QObject::trUtf8( "parcely" ),  Parcely );
  controls.formComboBox->addItem( QObject::trUtf8( "budovy" ),  Budovy );
  controls.formComboBox->addItem( QObject::trUtf8( "jednotky" ),  Jednotky );

  connect( controls.formComboBox, SIGNAL( activated( int ) ), this, SLOT( changeForm( int ) ) );
  connect( controls.searchButton, SIGNAL( clicked() ), this, SLOT( search() ) );

  //  forms.vlastnici->vlastniciSearchEnabled();
//  initComboBoxModels();

  controls.searchForms->setCurrentIndex( 0 );

  connect( forms.vlastnici, SIGNAL( searchEnabled( bool ) ), controls.searchButton, SLOT( setEnabled( bool ) ) );
  connect( forms.parcely, SIGNAL( searchEnabled( bool ) ), controls.searchButton, SLOT( setEnabled( bool ) ) );

  initForms();
}

void SearchFormController::setConnectionName(const QString &connectionName)
{
  mConnectionName = connectionName;
  initComboBoxModels();
}

void SearchFormController::search()
{
  Form form = Form( controls.formComboBox->itemData( controls.formComboBox->currentIndex() ).toInt() );
  QApplication::setOverrideCursor( Qt::WaitCursor );
  qApp->processEvents();
  switch( form )
  {
  case Parcely:
    searchParcely();
    break;
  case Budovy:
    searchBudovy();
    break;
  case Jednotky:
    searchJednotky();
    break;
  case Vlastnici:
    searchVlastnici();
    break;

  default:
    break;
  }
  QApplication::restoreOverrideCursor();
}


void SearchFormController::changeForm(int index)
{
  controls.searchForms->setCurrentIndex( index );
  initForms();
}

void SearchFormController::searchVlastnici()
{
  QString jmeno = forms.vlastnici->jmeno();
  QString rcIco = forms.vlastnici->rcIco();
  QString lv = forms.vlastnici->lv();
  bool sjm = forms.vlastnici->isSjm();
  bool opo = forms.vlastnici->isOpo();
  bool ofo = forms.vlastnici->isOfo();


  QUrl url = QString( "showText?page=search&type=vlastnici&jmeno=%1&rcIco=%2&sjm=%3&opo=%4&ofo=%5&lv=%6" )
      .arg( jmeno ).arg( rcIco ).arg( sjm ? 1 : 0 ).arg( opo ? 1 : 0 ).arg( ofo ? 1 : 0 ).arg( lv );
  emit actionTriggered( url );
}

void SearchFormController::searchParcely()
{
  QString parcelniCislo = forms.parcely->parcelniCislo();
  int typ = forms.parcely->typParcely();
  QString druh = forms.parcely->druhPozemkuKod();
  QString lv = forms.parcely->lv();

  QUrl url = QString( "showText?page=search&type=parcely&parcelniCislo=%1&typ=%2&druh=%3&lv=%4" )
      .arg( parcelniCislo ).arg( typ ).arg( druh ).arg( lv );
  emit actionTriggered( url );
}

void SearchFormController::searchBudovy()
{
  QString domovniCislo = forms.budovy->domovniCislo();
  QString naParcele = forms.budovy->naParcele();
  QString zpusobVyuziti = forms.budovy->zpusobVyuzitiKod();
  QString lv = forms.budovy->lv();

  QUrl url = QString( "showText?page=search&type=budovy&domovniCislo=%1&naParcele=%2&zpusobVyuziti=%3&lv=%4" )
      .arg( domovniCislo ).arg( naParcele ).arg( zpusobVyuziti ).arg( lv );
  qDebug()<< url;
  emit actionTriggered( url );
}

void SearchFormController::searchJednotky()
{
  QString cisloJednotky = forms.jednotky->cisloJednotky();
  QString domovniCislo = forms.jednotky->domovniCislo();
  QString naParcele = forms.jednotky->naParcele();
  QString zpusobVyuziti = forms.jednotky->zpusobVyuzitiKod();
  QString lv = forms.jednotky->lv();

  QUrl url = QString( "showText?page=search&type=jednotky&"
                      "cisloJednotky=%1&domovniCislo=%2&naParcele=%3&zpusobVyuziti=%4&lv=%5" )
      .arg( cisloJednotky ).arg( domovniCislo ).arg( naParcele ).arg( zpusobVyuziti ).arg( lv );
  qDebug()<< url;
  emit actionTriggered( url );
}

void SearchFormController::initForms()
{
  forms.vlastnici->postInit();
  forms.parcely->postInit();
  forms.budovy->postInit();

  VlastniciSearchForm *f = controls.searchForms->currentWidget()->findChild<VlastniciSearchForm*>();
  if ( f )
  {
    f->postInit();
  }
}

void SearchFormController::initComboBoxModels()
{
  mDruhParcely = new VfkTableModel( mConnectionName, this );
  mDruhParcely->druhyPozemku();
  mDruhPozemkoveParcely = new VfkTableModel( mConnectionName, this );
  mDruhPozemkoveParcely->druhyPozemku( true, false );
  mDruhStavebniParcely = new VfkTableModel( mConnectionName, this );
  mDruhStavebniParcely->druhyPozemku( false, true );
  mZpusobVyuzitiBudovy = new VfkTableModel( mConnectionName, this );
  mZpusobVyuzitiBudovy->zpusobVyuzitiBudov();
  mZpusobVyuzitiJednotek = new VfkTableModel( mConnectionName, this );
  mZpusobVyuzitiJednotek->zpusobVyuzitiJednotek();


  QString falseKodForDefaultDruh = "";
  QString text = trUtf8( "libovolný" );
  QList<QString> fakeRow;
  fakeRow << falseKodForDefaultDruh << text;

  forms.parcely->setDruhPozemkuModel( addFirstRowToModel( mDruhParcely, fakeRow ) );
  forms.parcely->setDruhPozemkuPozemkovaModel( addFirstRowToModel( mDruhPozemkoveParcely, fakeRow ) );
  forms.parcely->setDruhPozemkuStavebniModel( mDruhStavebniParcely );
  forms.budovy->setZpusobVyuzitiModel( addFirstRowToModel( mZpusobVyuzitiBudovy, fakeRow ) );
  forms.jednotky->setZpusobVyuzitiModel( addFirstRowToModel( mZpusobVyuzitiJednotek, fakeRow ) );
}

QStandardItemModel *SearchFormController::addFirstRowToModel(QAbstractItemModel *oldModel, QList<QString> newRow)
{
  QStandardItemModel *model = new QStandardItemModel( this );
  QList< QStandardItem * > items;

  foreach ( QString str, newRow )
  {
    items << new QStandardItem( str );
  }
  model->appendRow(items);

  for ( int i = 0; i < oldModel->rowCount(); i++ )
  {
    QList< QStandardItem * > items;
    for ( int j = 0; j < oldModel->columnCount(); j++ )
    {
      QModelIndex index = oldModel->index(i, j);
      QVariant data = oldModel->data( index );
      QStandardItem *item = new QStandardItem( data.toString() );
      items << item;
    }
    model->appendRow( items );
  }
  return model;
}

