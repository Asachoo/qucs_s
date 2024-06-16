/*
 * quarterwave_filter.cpp - Quarter wavelength filter implementation
 *
 * copyright (C) 2015 Andres Martinez-Mera <andresmartinezmera@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "quarterwave_filter.h"

#include <QString>
#include <QMessageBox>

double QuarterWave_Filter::bw = 0;
double QuarterWave_Filter::fc = 0;
double QuarterWave_Filter::d_lamdba4 = 0;

QuarterWave_Filter::QuarterWave_Filter()
{
}

QString QuarterWave_Filter::getLineString(bool isMicrostrip, double width_or_impedance, double l, int x, int y, int rotate)
{
  if (isMicrostrip)
  {
     return QString("<MLIN MS1 1 %1 %2 26 -30 0 %3 \"Sub1\" 1 \"%4mm\" 1 \"%5mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"26.85\" 0>\n")
               .arg(x).arg(y).arg(rotate).arg(width_or_impedance*1000).arg(l*1000);
  }
  else
  {
     return QString("<TLIN Line1 1 %1 %2 -26 20 0 %3 \"%4\" 1 \"%5\" 1 \"0 dB\" 0 \"26.85\" 0>\n")
            .arg(x).arg(y).arg(rotate).arg(width_or_impedance).arg(l);
  }
}

QString QuarterWave_Filter::getWireString(int x1, int y1, int x2, int y2)
{
  return QString("<%1 %2 %3 %4 \"\" 0 0 0>\n").arg(x1).arg(y1).arg(x2).arg(y2);
}

QString QuarterWave_Filter::getTeeString(int x, int y, double width1, double width2, double width3)
{
  return QString("<MTEE MS1 1 %1 %2 -26 20 1 0 \"Sub1\" 1 \"%3mm\" 1 \"%4mm\" 1 \"%5mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"26.85\" 0>\n")
    .arg(x).arg(y).arg(width1*1000).arg(width2*1000).arg(width3*1000);
}

// -----------------------------------------------------------------------
QString *QuarterWave_Filter::createSchematic(tFilter *Filter, tSubstrate *Substrate, bool isMicrostrip)
{
  if (Filter->Class < 2)
  {
      QMessageBox::warning(0, QObject::tr("Error"),
                            QObject::tr("Quarter wave filters do not allow low-pass nor high-pass masks\n"));
      return NULL;
  }
  // Auxiliary variables
  double Zres;
  double W_line, W_res, er_eff_line, er_eff_res, L_line, L_res;
  double Z0 = Filter->Impedance; // System impedance

  // Set filter main params as static members
  fc = Filter->Frequency + 0.5 * (Filter->Frequency2 - Filter->Frequency);
  d_lamdba4 = 0.25 * LIGHTSPEED / fc;
  QuarterWave_Filter::bw = (Filter->Frequency2 - Filter->Frequency) / (fc);
  // create the Qucs schematic
  QString *s = new QString("<Qucs Schematic " PACKAGE_VERSION ">\n");
  QString c_s = "<Components>\n";
  QString w_s = "<Wires>\n";
  int x = 60;
  int x_space = 50;

  // First power and ground
  c_s += QString("<Pac P1 1 %1 330 18 -26 0 1 \"1\" 1 \"%2 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0>\n").arg(x).arg(Filter->Impedance);
  c_s += QString("<GND * 1 %1 360 0 0 0 0>\n").arg(x);
  w_s += getWireString(60, 180, 60, 300);
  w_s += getWireString(60, 180, 90, 180);
  x += 60;

  // If the microstrip implementation is selected, calculate the width of the Z0 lines.
  if (isMicrostrip)
      TL_Filter::getMicrostrip(Z0, fc, Substrate, W_line, er_eff_line); // Series line

  for (int i = 0; i < Filter->Order; i++)
  {
      // Calculate the impedance of the resonator
      if (Filter->Class == CLASS_BANDPASS)
            Zres = (pi*Z0*bw)/(4*getNormValue(i, Filter)); // Bandpass
      else
            Zres = (4*Z0)/(pi*bw*getNormValue(i, Filter)); // Bandstop

      if (isMicrostrip)
      {
          // Calculate width of the resonator according to its Z
          TL_Filter::getMicrostrip(Zres, fc, Substrate, W_res, er_eff_res); // Shunt QW resonator

          L_line = d_lamdba4/sqrt(er_eff_line); // Length of the line
          L_res = d_lamdba4/sqrt(er_eff_res); // Length of the resonator

          c_s += getLineString(isMicrostrip, W_line, L_line, x, 180, 0); // Series line
          c_s += getTeeString(x+ 60 + x_space, 180, W_line, W_line, W_res);
          c_s += getLineString(isMicrostrip, W_res, L_res, x+60 + x_space, 60, 3); // Shunt quarter-wavelength resonator

          w_s += getWireString(x+30, 180, x+30+x_space, 180);
          w_s += getWireString(x+60 + x_space, 90, x+60 + x_space, 150);
          w_s += getWireString(x+60 + x_space + 30, 180, x+60 + x_space + 30 + x_space, 180);
      }
      else
      {// Ideal transmission lines
          c_s += getLineString(isMicrostrip, Z0, d_lamdba4, x, 180, 0); // Series transmission line
          c_s += getLineString(isMicrostrip, Zres, d_lamdba4,  x+60+x_space, 60, 3); // Shunt quarter-wavelength resonator

          w_s += getWireString(x+30, 180, x+90+2*x_space, 180);// Join series lines
          w_s += getWireString(x+60 + x_space, 90, x+60 + x_space, 180); // Join middle point with the stub
      }
      if (Filter->Class == CLASS_BANDPASS) // If bandpass, shunt resonators
          c_s += QString("<GND * 1 %1 30 0 0 1 0>\n").arg(x + 60 + x_space);

    x += 120 + 2 * x_space;
  }

  // Last Z0 line
  if (isMicrostrip)
  {
      c_s += getLineString(isMicrostrip, W_line, d_lamdba4/sqrt(er_eff_line), x, 180);
  }
  else
  {// Ideal transmission line
      c_s += getLineString(isMicrostrip, Filter->Impedance, d_lamdba4, x, 180);
  }

  // Last power and ground
  x += 80;
  c_s += QString("<Pac P2 1 %1 330 18 -26 0 1 \"2\" 1 \"%2 Ohm\" 1 \"0 dBm\" 0 \"1 GHz\" 0>\n").arg(x).arg(Filter->Impedance);
  c_s += QString("<GND * 1 %1 360 0 0 0 0>\n").arg(x);
  w_s += getWireString(x, 180, x, 300);
  w_s += getWireString(x-50, 180, x, 180);
  // Components footer
  c_s += QString("<.SP SP1 1 70 460 0 67 0 0 \"lin\" 1 \"%2Hz\" 1 \"%3Hz\" 1 \"300\" 1 \"no\" 0 \"1\" 0 \"2\" 0>\n").arg(num2str(0.1 * Filter->Frequency)).arg(num2str(10.0 * Filter->Frequency));
  if (isMicrostrip)
    c_s += QString("<SUBST Sub1 1 300 500 -30 24 0 0 \"%1\" 1 \"%2m\" 1 \"%3m\" 1 \"%4\" 1 \"%5\" 1 \"%6\" 1>\n").arg(Substrate->er).arg(num2str(Substrate->height)).arg(num2str(Substrate->thickness)).arg(Substrate->tand).arg(Substrate->resistivity).arg(Substrate->roughness);
  c_s += QString("<Eqn Eqn1 1 450 560 -28 15 0 0 \"S21_dB=dB(S[2,1])\" 1 \"S11_dB=dB(S[1,1])\" 1 \"yes\" 0>\n");
  *s += c_s + "</Components>\n";
  *s += w_s + "</Wires>\n";
  // Footer
  *s += "<Diagrams>\n";
  *s += "</Diagrams>\n";
  *s += "<Paintings>\n";
  *s += QString("<Text 420 460 12 #000000 0 \"Quarter wave bandpass filter \\n ");
  switch (Filter->Type)
  {
    case TYPE_BESSEL: *s += QString("Bessel"); break;
    case TYPE_BUTTERWORTH: *s += QString("Butterworth"); break;
    case TYPE_CHEBYSHEV: *s += QString("Chebyshev"); break;
  }
  *s += QString(" %1Hz...%2Hz \\n ").arg(num2str(Filter->Frequency)).arg(num2str(Filter->Frequency2));
  *s += QString("Impedance matching %3 Ohm\">\n").arg(Filter->Impedance);
  *s += "</Paintings>\n";
  return s;
}
