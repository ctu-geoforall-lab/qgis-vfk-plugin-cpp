#include "jednotkysearchform.h"

JednotkySearchForm::JednotkySearchForm(QWidget *parent) :
  QWidget(parent)
{
  setupUi(this);
}

void JednotkySearchForm::postInit()
{
}

void JednotkySearchForm::setZpusobVyuzitiModel(QAbstractItemModel *model)
{
  mZpusobVyuzitiModel = model;
  mZpVyuzitiCombo->setModel( model );
  mZpVyuzitiCombo->setModelColumn( 1 ); // FIXME: magic number
}

QString JednotkySearchForm::zpusobVyuzitiKod()
{
  int row = mZpVyuzitiCombo->currentIndex();
  QModelIndex index = mZpVyuzitiCombo->model()->index( row, 0 ); // FIXME: magic number
  return mZpVyuzitiCombo->model()->data( index ).toString();
}

