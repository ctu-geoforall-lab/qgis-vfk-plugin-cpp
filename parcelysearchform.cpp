#include "parcelysearchform.h"

ParcelySearchForm::ParcelySearchForm(QWidget *parent) :
  QWidget(parent)
{
  setupUi(this);

  connect( parCisloLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( parcelySearchEnabled() ) );

  connect(  typParcelyCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setDruhModel() ) );

  QRegExp rx( "[0-9]*/?[0-9]*" );
  QValidator *validator = new QRegExpValidator( rx, this );
  parCisloLineEdit->setValidator( validator );
}

void ParcelySearchForm::postInit()
{
  parcelySearchEnabled();
}

void ParcelySearchForm::setDruhPozemkuModel( QAbstractItemModel *model )
{
  defaultModel = pozemkovaModel = stavebniModel = model;
  druhPozemkuCombo->setModel( model );
  druhPozemkuCombo->setModelColumn( 1 ); // FIXME: magic number
}

void ParcelySearchForm::parcelySearchEnabled()
{
  emit searchEnabled( true );
//  if ( parCisloLineEdit->text().isEmpty() )
//  {
//    emit searchEnabled( false );
//  }
//  else
//  {
//    emit searchEnabled( true );
  //  }
}

void ParcelySearchForm::setDruhModel()
{
  if ( typParcelyCombo->currentIndex() == 1 )
    druhPozemkuCombo->setModel( pozemkovaModel );
  else if ( typParcelyCombo->currentIndex() == 2 )
    druhPozemkuCombo->setModel( stavebniModel );
  else
    druhPozemkuCombo->setModel( defaultModel );
}

void ParcelySearchForm::setDruhPozemkuStavebniModel(QAbstractItemModel *model)
{
  stavebniModel = model;
}

void ParcelySearchForm::setDruhPozemkuPozemkovaModel(QAbstractItemModel *model)
{
  pozemkovaModel = model;
}

int ParcelySearchForm::typParcely()
{
  return typParcelyCombo->currentIndex();
}

QString ParcelySearchForm::druhPozemkuKod()
{
  int row = druhPozemkuCombo->currentIndex();
  QModelIndex index = druhPozemkuCombo->model()->index( row, 0 ); // FIXME: magic number
  return druhPozemkuCombo->model()->data( index ).toString();
}
