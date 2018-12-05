/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




/*
The file defines some classes for transformation of PDF content stream.
*/


#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <sstream>
#include <string>
#include <math.h>
#include <vector>
#include <iostream>
#include "Utils.h"

namespace merge_lib
{
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

   class TransformationMatrix
   {
   public:
      TransformationMatrix(double a = 1, double b = 0, double c = 0, double d = 1, double e = 0, double f = 0):
         _a(a), _b(b), _c(c), _d(d), _e(e), _f(f)
         {}
         TransformationMatrix(const TransformationMatrix & copy)
         {
            setParameters(copy._a, copy._b, copy._c, copy._d, copy._e, copy._f);
         }
         void setParameters(double a, double b, double c, double d, double e, double f)
         {
            _a = Utils::normalizeValue(a);
            _b = Utils::normalizeValue(b);
            _c = Utils::normalizeValue(c);
            _d = Utils::normalizeValue(d);
            _e = Utils::normalizeValue(e);
            _f = Utils::normalizeValue(f);
         }
         void add(const TransformationMatrix & tm)
         {
            double newA = _a*tm._a + _b*tm._c;
            double newB = _a*tm._b + _b*tm._d;
            double newC = _c*tm._a + _d*tm._c;
            double newD = _c*tm._b + _d*tm._d;
            double newE = _e*tm._a + _f*tm._c + tm._e;
            double newF = _e*tm._b + _f*tm._d + tm._f;

            // we need to round the values to avoid not-needed transformation
            // since 1.e-17 is not 0 from PDF point of view, while such double
            // value really means 0.
            _a = Utils::normalizeValue(newA);
            _b = Utils::normalizeValue(newB);
            _c = Utils::normalizeValue(newC);
            _d = Utils::normalizeValue(newD);
            _e = Utils::normalizeValue(newE);
            _f = Utils::normalizeValue(newF);        
         }
         std::string getValue()
         {
            std::ostringstream value;

            value << "[ " << _a << " " << _b << " " << _c << " " << _d << " " << _e << " " << _f << " ]\n";
            return value.str();

         }
         std::string getCMT()
         {
            std::ostringstream buf;      
            buf << std::fixed << _a <<" "<< _b <<" "<< _c <<" "<< _d << " "<< _e << " "<< _f << " cm\n";
            return buf.str();
         }
         void recalculateCoordinates(double & x, double &y)
         {
            double inputX = x;
            double inputY = y;
            x = _a*inputX + _c*inputY + _e;
            y = _b*inputX + _d*inputY + _f;
         }
   private:
      double _a, _b, _c, _d, _e, _f;
   };

   // base class of transformation CMT
   class Transformation
   {
   public:
      Transformation(): _tm(){};
      virtual Transformation * getClone() const = 0;
      std::string getCMT()
      {
         return _tm.getCMT();
      }
      virtual ~Transformation() {};
      const TransformationMatrix & getMatrix()
      {
         return _tm;
      }
      void addMatrix(const TransformationMatrix & tm)
      {
         _tm.add(tm);
      }
   protected:
      TransformationMatrix _tm;
   };

   // rotation CMT
   class Rotation: public Transformation
   {
   public:
      Rotation(double angle):Transformation(),_angle(angle)
      {
         double cosValue = cos(_angle * (M_PI / 180));
         double sinValue = sin(_angle * (M_PI / 180));

         _tm.setParameters(cosValue, sinValue, -sinValue, cosValue, 0, 0);

      };
      virtual ~Rotation(){};
      virtual Transformation * getClone() const
      {
         return new Rotation(_angle);
      }   

   protected:
      double _angle;   // number of degrees to rotate
   };

   // translation CMT
   class Translation: public Transformation
   {
   public:
      Translation(double x, double y):Transformation(),_x(x),_y(y)
      {
         _tm.setParameters(1, 0, 0, 1, _x, _y);
      };
      virtual ~Translation(){};
      virtual Transformation * getClone() const
      {
         return new Translation(_x, _y);
      }   
   protected:
      double _x;
      double _y;
   };

   // scaling CMT
   class Scaling: public Transformation
   {
   public:
      Scaling(double x):Transformation(),_x(x)
      {      
         _tm.setParameters(_x, 0, 0, _x, 0, 0);
      };
      virtual Transformation * getClone() const
      {
         return new Scaling(_x);
      }   
   protected:
      double _x; // the value to multiply the content
   };



   // transformation can consist of one or several 
   // operations like rotation, scaling, translation
   typedef std::vector<Transformation *> PageTransformations;

   // This is interface class for setting transformation parameters
   // 
   class TransformationDescription
   {
   public:
      TransformationDescription( double x = 0,  // leftBottomX coordinate
         double y = 0,  // leftBottomY coordinate
         double scale = 1, // scale (by default = 1 = NONE
         int angel = 0): // rotation (0,90,180,270)
      _x(x),_y(y),_scale(scale),_angel(angel)
      {
         if( _angel )
         {
            _transforms.push_back(new Rotation(_angel));
         }

         if( !Utils::doubleEquals(_scale,1) && !Utils::doubleEquals(_scale,0) )
         {
            _transforms.push_back(new Scaling(_scale));
         }
      }
      virtual ~TransformationDescription()
      {
         for(size_t i = 0;i<_annotsTransforms.size();i++)
         {
            if( _annotsTransforms[i] )
            {
               delete _annotsTransforms[i];
               _annotsTransforms[i] = 0;
            }
            _annotsTransforms.clear();
         }
         for(size_t i = 0;i<_transforms.size();i++)
         {
            if( _transforms[i] )
            {
               delete _transforms[i];
               _transforms[i] = 0;
            }
         }
         _transforms.clear();
      }
      void addRotation(int rotation)
      {
         if( rotation )
         {
            _angel = (_angel - rotation)%360;
            // /Rotation rotate the object, while _angel rotate the coordinate system
            // where object is located, that's why 
            // we should compensate that
            _transforms.push_back(new Rotation(360-rotation));
         }
      }
      const PageTransformations & getTransformations() const
      {
         return _transforms;
      }
      const PageTransformations getAnnotsTransformations() const
      {
           PageTransformations trans;
           trans = _transforms;
           for(size_t i = 0; i < _annotsTransforms.size(); ++i)
           {
              trans.push_back(_annotsTransforms[i]);
           }
           return trans;
      }
      void addAnnotsTransformation( Transformation & trans )
      {
         _annotsTransforms.push_back(trans.getClone());
      }

      // method recalculates the final translation in order to put 
      // object into needed x,y coordinates.
      // Page is located from position 0,0
      void recalculateTranslation(double width, double height)
      {
         double dx1 = 0;
         double dy1 = 0;

         double scaling = ( Utils::doubleEquals(_scale,0))?1:_scale;

         switch(_angel)
         {
         case 0: 
            dx1 = _x/scaling;
            dy1 = _y/scaling;
            break;
         case -270:
         case  90:
            dx1 = _y/scaling ;
            dy1 = - _x /scaling - height;
            break;
         case 180:
         case -180:
            dx1 = - _x /scaling - width;
            dy1 = - _y /scaling - height;
            break;
         case 270:
         case -90:
            dx1 = - _y/scaling - width;
            dy1 = _x/scaling;
            break;
         default:
            std::cerr<<"Unsupported rotation parameter"<<_angel<<std::endl;
            break;
         }

         //std::cerr<< "dx1 = "<<dx1<<"dy1 = "<<dy1<<std::endl;
         if( ! (Utils::doubleEquals(dx1,0) && Utils::doubleEquals(dy1,0)) )
         {
            // This translation is needed to put transformed content into
            // desired coordinates
            _transforms.push_back(new Translation(dx1,dy1));
         }
      }
      TransformationDescription( const TransformationDescription & copy)
      {
         *this = copy;
      }
      TransformationDescription& operator = (const TransformationDescription &copy)
      {
         if( this != &copy )
         {
            for(size_t i = 0;i < copy._annotsTransforms.size();i++)
            {
               _annotsTransforms.push_back(copy._annotsTransforms[i]->getClone());
            }
            for(size_t i = 0; i < copy._transforms.size(); ++i)
            {
               _transforms.push_back(copy._transforms[i]->getClone());
            }
            _x = copy._x;
            _y = copy._y;
            _scale = copy._scale;
            _angel = copy._angel;
         }
         return *this;
      }
      std::string getCMT()
      {
         std::stringstream content;
         for(size_t i = 0;i<_transforms.size();i++)
         {
            content<<_transforms[i]->getCMT();
         }
         return content.str();
      }


   private:
      double _x;
      double _y;
      double _scale;
      int _angel;

      PageTransformations _transforms;
      PageTransformations _annotsTransforms;
   };

}
#endif
