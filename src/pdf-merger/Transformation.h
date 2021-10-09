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
         m_a(a), m_b(b), m_c(c), m_d(d), m_e(e), m_f(f)
         {}
         TransformationMatrix(const TransformationMatrix & copy)
         {
            setParameters(copy.m_a, copy.m_b, copy.m_c, copy.m_d, copy.m_e, copy.m_f);
         }
         void setParameters(double a, double b, double c, double d, double e, double f)
         {
            m_a = Utils::normalizeValue(a);
            m_b = Utils::normalizeValue(b);
            m_c = Utils::normalizeValue(c);
            m_d = Utils::normalizeValue(d);
            m_e = Utils::normalizeValue(e);
            m_f = Utils::normalizeValue(f);
         }
         void add(const TransformationMatrix & tm)
         {
            double newA = m_a*tm.m_a + m_b*tm.m_c;
            double newB = m_a*tm.m_b + m_b*tm.m_d;
            double newC = m_c*tm.m_a + m_d*tm.m_c;
            double newD = m_c*tm.m_b + m_d*tm.m_d;
            double newE = m_e*tm.m_a + m_f*tm.m_c + tm.m_e;
            double newF = m_e*tm.m_b + m_f*tm.m_d + tm.m_f;

            // we need to round the values to avoid not-needed transformation
            // since 1.e-17 is not 0 from PDF point of view, while such double
            // value really means 0.
            m_a = Utils::normalizeValue(newA);
            m_b = Utils::normalizeValue(newB);
            m_c = Utils::normalizeValue(newC);
            m_d = Utils::normalizeValue(newD);
            m_e = Utils::normalizeValue(newE);
            m_f = Utils::normalizeValue(newF);
         }
         std::string getValue()
         {
            std::ostringstream value;

            value << "[ " << m_a << " " << m_b << " " << m_c << " " << m_d << " " << m_e << " " << m_f << " ]\n";
            return value.str();

         }
         std::string getCMT()
         {
            std::ostringstream buf;      
            buf << std::fixed << m_a <<" "<< m_b <<" "<< m_c <<" "<< m_d << " "<< m_e << " "<< m_f << " cm\n";
            return buf.str();
         }
         void recalculateCoordinates(double & x, double &y)
         {
            double inputX = x;
            double inputY = y;
            x = m_a*inputX + m_c*inputY + m_e;
            y = m_b*inputX + m_d*inputY + m_f;
         }
   private:
      double m_a, m_b, m_c, m_d, m_e, m_f;
   };

   // base class of transformation CMT
   class Transformation
   {
   public:
      Transformation(): m_tm(){};
      virtual Transformation * getClone() const = 0;
      std::string getCMT()
      {
         return m_tm.getCMT();
      }
      virtual ~Transformation() {};
      const TransformationMatrix & getMatrix()
      {
         return m_tm;
      }
      void addMatrix(const TransformationMatrix & tm)
      {
         m_tm.add(tm);
      }
   protected:
      TransformationMatrix m_tm;
   };

   // rotation CMT
   class Rotation: public Transformation
   {
   public:
      Rotation(double angle):Transformation(),m_angle(angle)
      {
         double cosValue = cos(m_angle * (M_PI / 180));
         double sinValue = sin(m_angle * (M_PI / 180));

         m_tm.setParameters(cosValue, sinValue, -sinValue, cosValue, 0, 0);

      };
      virtual ~Rotation(){};
      virtual Transformation * getClone() const
      {
         return new Rotation(m_angle);
      }   

   protected:
      double m_angle;   // number of degrees to rotate
   };

   // translation CMT
   class Translation: public Transformation
   {
   public:
      Translation(double x, double y):Transformation(),m_x(x),m_y(y)
      {
         m_tm.setParameters(1, 0, 0, 1, m_x, m_y);
      };
      virtual ~Translation(){};
      virtual Transformation * getClone() const
      {
         return new Translation(m_x, m_y);
      }   
   protected:
      double m_x;
      double m_y;
   };

   // scaling CMT
   class Scaling: public Transformation
   {
   public:
      Scaling(double x):Transformation(),m_x(x)
      {      
         m_tm.setParameters(m_x, 0, 0, m_x, 0, 0);
      };
      virtual Transformation * getClone() const
      {
         return new Scaling(m_x);
      }   
   protected:
      double m_x; // the value to multiply the content
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
      m_x(x),m_y(y),m_scale(scale),m_angel(angel)
      {
         if( m_angel )
         {
            m_transforms.push_back(new Rotation(m_angel));
         }

         if( !Utils::doubleEquals(m_scale,1) && !Utils::doubleEquals(m_scale,0) )
         {
            m_transforms.push_back(new Scaling(m_scale));
         }
      }
      virtual ~TransformationDescription()
      {
         for(size_t i = 0;i<m_annotsTransforms.size();i++)
         {
            if( m_annotsTransforms[i] )
            {
               delete m_annotsTransforms[i];
               m_annotsTransforms[i] = 0;
            }
            m_annotsTransforms.clear();
         }
         for(size_t i = 0;i<m_transforms.size();i++)
         {
            if( m_transforms[i] )
            {
               delete m_transforms[i];
               m_transforms[i] = 0;
            }
         }
         m_transforms.clear();
      }
      void addRotation(int rotation)
      {
         if( rotation )
         {
            m_angel = (m_angel - rotation)%360;
            // /Rotation rotate the object, while m_angel rotate the coordinate system
            // where object is located, that's why 
            // we should compensate that
            m_transforms.push_back(new Rotation(360-rotation));
         }
      }
      const PageTransformations & getTransformations() const
      {
         return m_transforms;
      }
      const PageTransformations getAnnotsTransformations() const
      {
           PageTransformations trans;
           trans = m_transforms;
           for(size_t i = 0; i < m_annotsTransforms.size(); ++i)
           {
              trans.push_back(m_annotsTransforms[i]);
           }
           return trans;
      }
      void addAnnotsTransformation( Transformation & trans )
      {
         m_annotsTransforms.push_back(trans.getClone());
      }

      // method recalculates the final translation in order to put 
      // object into needed x,y coordinates.
      // Page is located from position 0,0
      void recalculateTranslation(double width, double height)
      {
         double dx1 = 0;
         double dy1 = 0;

         double scaling = ( Utils::doubleEquals(m_scale,0))?1:m_scale;

         switch(m_angel)
         {
         case 0: 
            dx1 = m_x/scaling;
            dy1 = m_y/scaling;
            break;
         case -270:
         case  90:
            dx1 = m_y/scaling ;
            dy1 = - m_x /scaling - height;
            break;
         case 180:
         case -180:
            dx1 = - m_x /scaling - width;
            dy1 = - m_y /scaling - height;
            break;
         case 270:
         case -90:
            dx1 = - m_y/scaling - width;
            dy1 = m_x/scaling;
            break;
         default:
            std::cerr<<"Unsupported rotation parameter"<<m_angel<<std::endl;
            break;
         }

         //std::cerr<< "dx1 = "<<dx1<<"dy1 = "<<dy1<<std::endl;
         if( ! (Utils::doubleEquals(dx1,0) && Utils::doubleEquals(dy1,0)) )
         {
            // This translation is needed to put transformed content into
            // desired coordinates
            m_transforms.push_back(new Translation(dx1,dy1));
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
            for(size_t i = 0;i < copy.m_annotsTransforms.size();i++)
            {
               m_annotsTransforms.push_back(copy.m_annotsTransforms[i]->getClone());
            }
            for(size_t i = 0; i < copy.m_transforms.size(); ++i)
            {
               m_transforms.push_back(copy.m_transforms[i]->getClone());
            }
            m_x = copy.m_x;
            m_y = copy.m_y;
            m_scale = copy.m_scale;
            m_angel = copy.m_angel;
         }
         return *this;
      }
      std::string getCMT()
      {
         std::stringstream content;
         for(size_t i = 0;i<m_transforms.size();i++)
         {
            content<<m_transforms[i]->getCMT();
         }
         return content.str();
      }


   private:
      double m_x;
      double m_y;
      double m_scale;
      int m_angel;

      PageTransformations m_transforms;
      PageTransformations m_annotsTransforms;
   };

}
#endif
