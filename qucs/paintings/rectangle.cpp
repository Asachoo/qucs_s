/***************************************************************************
                              rectangle.cpp
                             ---------------
    begin                : Sat Nov 22 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rectangle.h"
#include "filldialog.h"
#include "schematic.h"

#include <QPainter>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include <cmath>

#include "misc.h"

qucs::Rectangle::Rectangle(bool _filled)
{
  Name = "Rectangle ";
  isSelected = false;
  Pen = QPen(QColor());
  Brush = QBrush(Qt::lightGray);
  filled = _filled;
  cx = cy = 0;
  x1 = x2 = 0;
  y1 = y2 = 0;
}

qucs::Rectangle::~Rectangle()
{
}

void qucs::Rectangle::paint(QPainter *painter) {
  painter->save();

  painter->setPen(Pen);
  if (filled) {
    painter->setBrush(Brush);
  }
  painter->drawRect(cx, cy, x2, y2);

  if (isSelected) {
    painter->setPen(QPen(Qt::darkGray,Pen.width()+5));
    painter->drawRect(cx, cy, x2, y2);
    painter->setPen(QPen(Qt::white, Pen.width(), Pen.style()));
    painter->drawRect(cx, cy, x2, y2);

    misc::draw_resize_handle(painter, QPoint{cx, cy});
    misc::draw_resize_handle(painter, QPoint{cx, cy + y2});
    misc::draw_resize_handle(painter, QPoint{cx + x2, cy});
    misc::draw_resize_handle(painter, QPoint{cx + x2, cy + y2});
  }
  painter->restore();
}

// --------------------------------------------------------------------------
void qucs::Rectangle::paintScheme(Schematic *p)
{
  p->PostPaintEvent(_Rect, cx, cy, x2, y2);
}

// --------------------------------------------------------------------------
void qucs::Rectangle::getCenter(int& x, int &y)
{
  x = cx+(x2>>1);
  y = cy+(y2>>1);
}

// --------------------------------------------------------------------------
// Sets the center of the painting to x/y.
void qucs::Rectangle::setCenter(int x, int y, bool relative)
{
  if(relative) { cx += x;  cy += y; }
  else { cx = x-(x2>>1);  cy = y-(y2>>1); }
}

// --------------------------------------------------------------------------
Painting* qucs::Rectangle::newOne()
{
  return new qucs::Rectangle();
}

// --------------------------------------------------------------------------
Element* qucs::Rectangle::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("Rectangle");
  BitmapFile = (char *) "rectangle";

  if(getNewOne)  return new qucs::Rectangle();
  return 0;
}

// --------------------------------------------------------------------------
Element* qucs::Rectangle::info_filled(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("filled Rectangle");
  BitmapFile = (char *) "filledrect";

  if(getNewOne)  return new qucs::Rectangle(true);
  return 0;
}

// --------------------------------------------------------------------------
bool qucs::Rectangle::load(const QString& s)
{
  bool ok;

  QString n;
  n  = s.section(' ',1,1);    // cx
  cx = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',2,2);    // cy
  cy = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',3,3);    // x2
  x2 = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',4,4);    // y2
  y2 = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',5,5);    // color
  QColor co = misc::ColorFromString(n);
  Pen.setColor(co);
  if(!Pen.color().isValid()) return false;

  n  = s.section(' ',6,6);    // thickness
  Pen.setWidth(n.toInt(&ok));
  if(!ok) return false;

  n  = s.section(' ',7,7);    // line style
  Pen.setStyle((Qt::PenStyle)n.toInt(&ok));
  if(!ok) return false;

  n  = s.section(' ',8,8);    // fill color
  co = misc::ColorFromString(n);
  Brush.setColor(co);
  if(!Brush.color().isValid()) return false;

  n  = s.section(' ',9,9);    // fill style
  Brush.setStyle((Qt::BrushStyle)n.toInt(&ok));
  if(!ok) return false;

  n  = s.section(' ',10,10);    // filled
  if(n.toInt(&ok) == 0) filled = false;
  else filled = true;
  if(!ok) return false;

  return true;
}

// --------------------------------------------------------------------------
QString qucs::Rectangle::save()
{
  QString s = Name +
	QString::number(cx) + " " + QString::number(cy) + " " +
	QString::number(x2) + " " + QString::number(y2) + " " +
	Pen.color().name() + " " + QString::number(Pen.width()) + " " +
	QString::number(Pen.style()) + " " +
	Brush.color().name() + " " + QString::number(Brush.style());
  if(filled) s += " 1";
  else s += " 0";
  return s;
}

// --------------------------------------------------------------------------
QString qucs::Rectangle::saveCpp()
{
  QString b = filled ?
    QString (", QBrush (QColor (\"%1\"), %2)").
    arg(Brush.color().name()).arg(toBrushString(Brush.style())) : "";
  QString s =
    QString ("new Area (%1, %2, %3, %4, "
	     "QPen (QColor (\"%5\"), %6, %7)%8)").
    arg(cx).arg(cy).arg(x2).arg(y2).
    arg(Pen.color().name()).arg(Pen.width()).arg(toPenString(Pen.style())).
    arg(b);
  s = "Rects.append (" + s + ");";
  return s;
}

QString qucs::Rectangle::saveJSON()
{
  QString b = filled ?
    QString ("\"colorfill\" : \"%1\", \"stylefill\" : \"%2\"").
    arg(Brush.color().name()).arg(toBrushString(Brush.style())) : "";

  QString s =
    QStringLiteral("{\"type\" : \"rectangle\", "
      "\"x\" : %1, \"y\" : %2, \"w\" : %3, \"h\" : %4, "
      "\"color\" : \"%5\", \"thick\" : %6, \"style\" : \"%7\", %8},").
      arg(cx).arg(cy).arg(x2).arg(y2).
      arg(Pen.color().name()).arg(Pen.width()).arg(toPenString(Pen.style())).
      arg(b);
  return s;
}

// --------------------------------------------------------------------------
// Checks if the resize area was clicked.
bool qucs::Rectangle::resizeTouched(float fX, float fY, float len)
{
  State = -1;

  QRectF r_ul(0,0,2*len,2*len);
  r_ul.moveCenter(QPointF(cx,cy));
  QRectF r_ll(0,0,2*len,2*len);
  r_ll.moveCenter(QPointF(cx, cy + y2));
  QRectF r_ur(0,0,2*len,2*len);
  r_ur.moveCenter(QPointF(cx + x2, cy));
  QRectF r_lr(0,0,2*len,2*len);
  r_lr.moveCenter(QPointF(cx + x2, cy + y2));

  if (!r_lr.contains(fX,fY) &&
      !r_ll.contains(fX,fY) &&
      !r_ur.contains(fX,fY) &&
      !r_ul.contains(fX,fY)) return false;

  if (r_lr.contains(fX,fY)) State = 0;
  if (r_ll.contains(fX,fY)) State = 1;
  if (r_ur.contains(fX,fY)) State = 2;
  if (r_ul.contains(fX,fY)) State = 3;

  return true;
}

// --------------------------------------------------------------------------
// Mouse move action during resize.
void qucs::Rectangle::MouseResizeMoving(int x, int y, Schematic *p)
{
  paintScheme(p);  // erase old painting
  switch(State) {
    case 0: x2 = x-cx; y2 = y-cy; // lower right corner
	    break;
    case 1: x2 -= x-cx; cx = x; y2 = y-cy; // lower left corner
	    break;
    case 2: x2 = x-cx; y2 -= y-cy; cy = y; // upper right corner
	    break;
    case 3: x2 -= x-cx; cx = x; y2 -= y-cy; cy = y; // upper left corner
	    break;
  }
  if(x2 < 0) { State ^= 1; x2 *= -1; cx -= x2; }
  if(y2 < 0) { State ^= 2; y2 *= -1; cy -= y2; }

  paintScheme(p);  // paint new painting
}

// --------------------------------------------------------------------------
// fx/fy are the precise coordinates, gx/gy are the coordinates set on grid.
// x/y are coordinates without scaling.
void qucs::Rectangle::MouseMoving(
	Schematic *paintScale, int, int, int gx, int gy,
	Schematic *p, int x, int y)
{
  if(State > 0) {
    if(State > 1)
      paintScale->PostPaintEvent(_Rect,x1, y1, x2-x1, y2-y1);  // erase old painting
    State++;
    x2 = gx;
    y2 = gy;
    paintScale->PostPaintEvent(_Rect,x1, y1, x2-x1, y2-y1);  // paint new rectangle
  }
  else { x2 = gx; y2 = gy; }

  cx = x;
  cy = y;
  p->PostPaintEvent(_Rect,cx+13, cy, 18, 12,0,0,true);  // paint new cursor symbol
  if(filled) {   // hatched ?
    p->PostPaintEvent(_Line, cx+14, cy+6, cx+19, cy+1,0,0,true);
    p->PostPaintEvent(_Line, cx+26, cy+1, cx+17, cy+10,0,0,true);
    p->PostPaintEvent(_Line, cx+29, cy+5, cx+24, cy+10,0,0,true);
  }
}

// --------------------------------------------------------------------------
bool qucs::Rectangle::MousePressing(Schematic *sch)
{
  Q_UNUSED(sch)
  State++;
  if(State == 1) {
    x1 = x2;
    y1 = y2;    // first corner is determined
  }
  else {
    if(x1 < x2) { cx = x1; x2 = x2-x1; } // cx/cy to upper left corner
    else { cx = x2; x2 = x1-x2; }
    if(y1 < y2) { cy = y1; y2 = y2-y1; }
    else { cy = y2; y2 = y1-y2; }
    x1 = y1 = 0;
    State = 0;
    return true;    // rectangle is ready
  }
  return false;
}

// --------------------------------------------------------------------------
// Checks if the coordinates x/y point to the painting.
bool qucs::Rectangle::getSelected(float fX, float fY, float w)
{
  if(filled) {
    QRectF r(cx,cy,x2,y2);
    if (!r.contains(fX,fY)) {
      return false;
    }
  } else {
    double sgn_x2 = std::copysign(1.0, x2);
    double sgn_y2 = std::copysign(1.0, y2);
    QRectF r_outer(cx - sgn_x2*w, cy - sgn_y2*w, // outer rectangle
                   x2 + 2*sgn_x2*w, y2 + 2*sgn_y2*w);
    QRectF r_inner(cx + sgn_x2*w, cy + sgn_y2*w,
                   x2 - 2*sgn_x2*w, y2 - 2*sgn_y2*w); // inner rectnagle w/o border
    if (r_outer.contains(fX,fY)) {
      if (r_inner.contains(fX,fY)) return false;
    } else {
      return false;
    }
  }

  return true;
}

// --------------------------------------------------------------------------
// Rotates around the center.
void qucs::Rectangle::rotate(int xc, int yc)
{
    int xr1 = cx - xc;
    int yr1 = cy - yc;
    int xr2 = cx + x2 - xc;
    int yr2 = cy + y2 - yc;

    int tmp = xr2;
    xr2  =  yr2;
    yr2  = -tmp;

    tmp = xr1;
    xr1  =  yr1;
    yr1  = -tmp;

    cx = xr1 + xc;
    cy = yr1 + yc;
    x2 = xr2 - xr1;
    y2 = yr2 - yr1;
}

// --------------------------------------------------------------------------
// Mirrors about center line.
void qucs::Rectangle::mirrorX()
{
  // nothing to do
}

// --------------------------------------------------------------------------
// Mirrors about center line.
void qucs::Rectangle::mirrorY()
{
  // nothing to do
}

// --------------------------------------------------------------------------
// Calls the property dialog for the painting and changes them accordingly.
// If there were changes, it returns 'true'.
bool qucs::Rectangle::Dialog(QWidget *parent)
{
  bool changed = false;

  FillDialog *d = new FillDialog(QObject::tr("Edit Rectangle Properties"), true, parent);
  misc::setPickerColor(d->ColorButt,Pen.color());
  d->LineWidth->setText(QString::number(Pen.width()));
  d->StyleBox->setCurrentIndex(Pen.style()-1);
  misc::setPickerColor(d->FillColorButt,Brush.color());
  d->FillStyleBox->setCurrentIndex(Brush.style());
  d->CheckFilled->setChecked(filled);
  d->slotCheckFilled(filled);

  if(d->exec() == QDialog::Rejected) {
    delete d;
    return false;
  }

  if(Pen.color() != misc::getWidgetBackgroundColor(d->ColorButt)) {
    Pen.setColor(misc::getWidgetBackgroundColor(d->ColorButt));
    changed = true;
  }
  if(Pen.width()  != d->LineWidth->text().toInt()) {
    Pen.setWidth(d->LineWidth->text().toInt());
    changed = true;
  }
  if(Pen.style()  != (Qt::PenStyle)(d->StyleBox->currentIndex()+1)) {
    Pen.setStyle((Qt::PenStyle)(d->StyleBox->currentIndex()+1));
    changed = true;
  }
  if(filled != d->CheckFilled->isChecked()) {
    filled = d->CheckFilled->isChecked();
    changed = true;
  }
  if(Brush.color() != misc::getWidgetBackgroundColor(d->FillColorButt)) {
    Brush.setColor(misc::getWidgetBackgroundColor(d->FillColorButt));
    changed = true;
  }
  if(Brush.style() != d->FillStyleBox->currentIndex()) {
    Brush.setStyle((Qt::BrushStyle)d->FillStyleBox->currentIndex());
    changed = true;
  }

  delete d;
  return changed;
}
