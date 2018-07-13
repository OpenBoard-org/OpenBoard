/*!
 * station-clock.js
 *
 * Copyright (c) 2010 Ruediger Appel
 * ludi at mac dot com
 *
 * Date: 2016-02-16
 * Version: 1.0.1
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Known Issues:
 *
 *   Shadows for some second hands is not on one layer
 * 
 * Thanks to Paul Schröfl for the Wiener Würfeluhr
 */

// clock body (UhrgehÃ¤use)
StationClock.NoBody         = 0;
StationClock.SmallWhiteBody = 1;
StationClock.RoundBody      = 2;
StationClock.RoundGreenBody = 3;
StationClock.SquareBody     = 4;
StationClock.ViennaBody     = 5;

// stroke dial (Zifferblatt)
StationClock.NoDial               = 0;
StationClock.GermanHourStrokeDial = 1;
StationClock.GermanStrokeDial     = 2;
StationClock.AustriaStrokeDial    = 3;
StationClock.SwissStrokeDial      = 4;
StationClock.ViennaStrokeDial     = 5;

//clock hour hand (Stundenzeiger)
StationClock.PointedHourHand = 1;
StationClock.BarHourHand     = 2;
StationClock.SwissHourHand   = 3;
StationClock.ViennaHourHand  = 4;

//clock minute hand (Minutenzeiger)
StationClock.PointedMinuteHand = 1;
StationClock.BarMinuteHand     = 2;
StationClock.SwissMinuteHand   = 3;
StationClock.ViennaMinuteHand  = 4;

//clock second hand (Sekundenzeiger)
StationClock.NoSecondHand            = 0;
StationClock.BarSecondHand           = 1;
StationClock.HoleShapedSecondHand    = 2;
StationClock.NewHoleShapedSecondHand = 3;
StationClock.SwissSecondHand         = 4;

// clock boss (Zeigerabdeckung)
StationClock.NoBoss     = 0;
StationClock.BlackBoss  = 1;
StationClock.RedBoss    = 2;
StationClock.ViennaBoss = 3;

// minute hand behavoir
StationClock.CreepingMinuteHand        = 0;
StationClock.BouncingMinuteHand        = 1;
StationClock.ElasticBouncingMinuteHand = 2;

// second hand behavoir
StationClock.CreepingSecondHand        = 0;
StationClock.BouncingSecondHand        = 1;
StationClock.ElasticBouncingSecondHand = 2;
StationClock.OverhastySecondHand       = 3;


function StationClock(clockId) {
  this.clockId = clockId; 
  this.radius  = 0;

  // hour offset
  this.hourOffset = 0;
  
  // clock body
  this.body              = StationClock.RoundBody;
  this.bodyShadowColor   = "rgba(0,0,0,0.5)";
  this.bodyShadowOffsetX = 0.03;
  this.bodyShadowOffsetY = 0.03;
  this.bodyShadowBlur    = 0.06;
  
  // body dial
  this.dial              = StationClock.GermanStrokeDial;
  this.dialColor         = 'rgb(60,60,60)';
  
  // clock hands
  this.hourHand          = StationClock.PointedHourHand;
  this.minuteHand        = StationClock.PointedMinuteHand;
  this.secondHand        = StationClock.HoleShapedSecondHand;
  this.handShadowColor   = 'rgba(0,0,0,0.3)';
  this.handShadowOffsetX = 0.03;
  this.handShadowOffsetY = 0.03;
  this.handShadowBlur    = 0.04;
	
	// clock colors
	this.hourHandColor     = 'rgb(0,0,0)';
	this.minuteHandColor   = 'rgb(0,0,0)';
	this.secondHandColor   = 'rgb(200,0,0)';
  
  // clock boss
  this.boss              = StationClock.NoBoss;
  this.bossShadowColor   = "rgba(0,0,0,0.2)";
  this.bossShadowOffsetX = 0.02;
  this.bossShadowOffsetY = 0.02;
  this.bossShadowBlur    = 0.03;
  
  // hand behavoir
  this.minuteHandBehavoir = StationClock.CreepingMinuteHand;
  this.secondHandBehavoir = StationClock.OverhastySecondHand;
  
  // hand animation
  this.minuteHandAnimationStep = 0;
  this.secondHandAnimationStep = 0;
  this.lastMinute = 0;
  this.lastSecond = 0;
};

StationClock.prototype.draw = function() {
  var clock = document.getElementById(this.clockId);
  if (clock) {
    var context = clock.getContext('2d');
    if (context) {
      this.radius = 0.75 * (Math.min(clock.width, clock.height) / 2);
      
      // clear canvas and set new origin
      context.clearRect(0, 0, clock.width, clock.height);
      context.save();
      context.translate(clock.width / 2, clock.height / 2);
      
      // draw body
      if (this.body != StationClock.NoStrokeBody) {
        context.save();
        switch (this.body) {
    		  case StationClock.SmallWhiteBody:
    		    this.fillCircle(context, "rgb(255,255,255)", 0, 0, 1);
    		    break;
          case StationClock.RoundBody:
            this.fillCircle(context, "rgb(255,255,255)", 0, 0, 1.1);
            context.save();
            this.setShadow(context, this.bodyShadowColor, this.bodyShadowOffsetX, this.bodyShadowOffsetY, this.bodyShadowBlur);
            this.strokeCircle(context, "rgb(0,0,0)", 0, 0, 1.1, 0.07);
            context.restore();
            break;
          case StationClock.RoundGreenBody:
            this.fillCircle(context, "rgb(235,236,212)", 0, 0, 1.1);
            context.save();
            this.setShadow(context, this.bodyShadowColor, this.bodyShadowOffsetX, this.bodyShadowOffsetY, this.bodyShadowBlur);
            this.strokeCircle(context, "rgb(180,180,180)", 0, 0, 1.1, 0.2);
            context.restore();
            this.strokeCircle(context, "rgb(29,84,31)", 0, 0, 1.15, 0.1);
            context.save();
            this.setShadow(context, "rgba(235,236,212,100)", -0.02, -0.02, 0.09);
            this.strokeCircle(context, 'rgb(76,128,110)', 0, 0, 1.1, 0.08);
            context.restore();
            break;
    		  case StationClock.SquareBody:
            context.save();
            this.setShadow(context, this.bodyShadowColor, this.bodyShadowOffsetX, this.bodyShadowOffsetY, this.bodyShadowBlur);
            this.fillSquare(context, 'rgb(237,235,226)', 0, 0, 2.4);
            this.strokeSquare(context, 'rgb(38,106,186)', 0, 0, 2.32, 0.16);
            context.restore();
      			context.save();
      			this.setShadow(context, this.bodyShadowColor, this.bodyShadowOffsetX, this.bodyShadowOffsetY, this.bodyShadowBlur);
            this.strokeSquare(context, 'rgb(42,119,208)', 0, 0, 2.24, 0.08);
      			context.restore();
      			break;
          case StationClock.ViennaBody:
            context.save();
            this.fillSymmetricPolygon(context, 'rgb(156,156,156)', [[-1.2,1.2],[-1.2,-1.2]],0.1);
            this.fillPolygon(context, 'rgb(156,156,156)', 0,1.2 , 1.2,1.2 , 1.2,0);
            this.fillCircle(context, 'rgb(255,255,255)', 0, 0, 1.05, 0.08);
            this.strokeCircle(context, 'rgb(0,0,0)', 0, 0, 1.05, 0.01);
            this.strokeCircle(context, 'rgb(100,100,100)', 0, 0, 1.1, 0.01);
            this.fillPolygon(context, 'rgb(100,100,100)', 0.45,1.2 , 1.2,1.2 , 1.2,0.45);
            this.fillPolygon(context, 'rgb(170,170,170)', 0.45,-1.2 , 1.2,-1.2 , 1.2,-0.45);
            this.fillPolygon(context, 'rgb(120,120,120)', -0.45,1.2 , -1.2,1.2 , -1.2,0.45);
            this.fillPolygon(context, 'rgb(200,200,200)', -0.45,-1.2 , -1.2,-1.2 , -1.2,-0.45);
            this.strokeSymmetricPolygon(context, 'rgb(156,156,156)', [[-1.2,1.2],[-1.2,-1.2]],0.01);
            this.fillPolygon(context, 'rgb(255,0,0)', 0.05,-0.6 , 0.15,-0.6 , 0.15,-0.45 , 0.05,-0.45);
            this.fillPolygon(context, 'rgb(255,0,0)', -0.05,-0.6 , -0.15,-0.6 , -0.15,-0.45 , -0.05,-0.45);
            this.fillPolygon(context, 'rgb(255,0,0)', 0.05,-0.35 , 0.15,-0.35 , 0.15,-0.30 ,  0.10,-0.20 , 0.05,-0.20);
            this.fillPolygon(context, 'rgb(255,0,0)', -0.05,-0.35 , -0.15,-0.35 , -0.15,-0.30 ,  -0.10,-0.20 , -0.05,-0.20);
            context.restore();
            break;
            }
          context.restore();
      }
      
      // draw dial
      for (var i = 0; i < 60; i++) {
        context.save();
        context.rotate(i * Math.PI / 30);
        switch (this.dial) {
          case StationClock.SwissStrokeDial:
            if ((i % 5) == 0) {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.75, 0.07);
            } else {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.92, 0.026);
            }
            break;
          case StationClock.AustriaStrokeDial:
            if ((i % 5) == 0) {
              this.fillPolygon(context, this.dialColor, -0.04, -1.0, 0.04, -1.0, 0.03, -0.78, -0.03, -0.78);
            } else {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.94, 0.02);
            }
            break;
          case StationClock.GermanStrokeDial:
          	if ((i % 15) == 0) {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.70, 0.08);
          	} else if ((i % 5) == 0) {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.76, 0.08);
          	} else {
          	  this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.92, 0.036);
          	}
          	break;
          case StationClock.GermanHourStrokeDial:
          	if ((i % 15) == 0) {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.70, 0.10);
          	} else if ((i % 5) == 0) {
              this.strokeLine(context, this.dialColor, 0.0, -1.0, 0.0, -0.74, 0.08);
          	}
          	break;
         case StationClock.ViennaStrokeDial:
            if ((i % 15) == 0) {
              this.fillPolygon(context, this.dialColor, 0.7,-0.1, 0.6,0, 0.7,0.1,  1,0.03,  1,-0.03);
            } else if ((i % 5) == 0) {
              this.fillPolygon(context, this.dialColor, 0.85,-0.06, 0.78,0, 0.85,0.06,  1,0.03,  1,-0.03);
            }
            this.fillCircle(context, this.dialColor, 0.0, -1.0, 0.03);
            break;
        }
        context.restore();
      }

      // get current time
      var time    = new Date();
      var millis  = time.getMilliseconds() / 1000.0;
      var seconds = time.getSeconds();
      var minutes = time.getMinutes();
      var hours   = time.getHours() + this.hourOffset;

      // draw hour hand
      context.save();
      context.rotate(hours * Math.PI / 6 + minutes * Math.PI / 360);
      this.setShadow(context, this.handShadowColor, this.handShadowOffsetX, this.handShadowOffsetY, this.handShadowBlur);
      switch (this.hourHand) {
        case StationClock.BarHourHand:
          this.fillPolygon(context, this.hourHandColor, -0.05, -0.6, 0.05, -0.6, 0.05, 0.15, -0.05, 0.15);
      	  break;
        case StationClock.PointedHourHand:
          this.fillPolygon(context, this.hourHandColor, 0.0, -0.6,  0.065, -0.53, 0.065, 0.19, -0.065, 0.19, -0.065, -0.53);
          break;
        case StationClock.SwissHourHand:
          this.fillPolygon(context, this.hourHandColor, -0.05, -0.6, 0.05, -0.6, 0.065, 0.26, -0.065, 0.26);
          break;
        case StationClock.ViennaHourHand:
          this.fillSymmetricPolygon(context, this.hourHandColor, [[-0.02,-0.72],[-0.08,-0.56],[-0.15,-0.45],[-0.06,-0.30],[-0.03,0],[-0.1,0.2],[-0.05,0.23],[-0.03,0.2]]);
      }
      context.restore();
      
      // draw minute hand
      context.save();
      switch (this.minuteHandBehavoir) {
        case StationClock.CreepingMinuteHand:
          context.rotate((minutes + seconds / 60) * Math.PI / 30);
  	      break;
        case StationClock.BouncingMinuteHand:
          context.rotate(minutes * Math.PI / 30);
  	      break;
        case StationClock.ElasticBouncingMinuteHand:
          if (this.lastMinute != minutes) {
            this.minuteHandAnimationStep = 3;
            this.lastMinute = minutes;
          }
          context.rotate((minutes + this.getAnimationOffset(this.minuteHandAnimationStep)) * Math.PI / 30);
          this.minuteHandAnimationStep--;
          break;
      }
      this.setShadow(context, this.handShadowColor, this.handShadowOffsetX, this.handShadowOffsetY, this.handShadowBlur);
      switch (this.minuteHand) {
        case StationClock.BarMinuteHand:
          this.fillPolygon(context, this.minuteHandColor, -0.05, -0.9, 0.035, -0.9, 0.035, 0.23, -0.05, 0.23);
      	  break;
        case StationClock.PointedMinuteHand:
          this.fillPolygon(context, this.minuteHandColor, 0.0, -0.93,  0.045, -0.885, 0.045, 0.23, -0.045, 0.23, -0.045, -0.885);
        	break;
        case StationClock.SwissMinuteHand:
        	this.fillPolygon(context, this.minuteHandColor, -0.035, -0.93, 0.035, -0.93, 0.05, 0.25, -0.05, 0.25);
        	break;
        case StationClock.ViennaMinuteHand:
          this.fillSymmetricPolygon(context, this.minuteHandColor, [[-0.02,-0.98],[-0.09,-0.7],[-0.03,0],[-0.05,0.2],[-0.01,0.4]]);
      }
      context.restore();
      
      // draw second hand
      context.save();
      switch (this.secondHandBehavoir) {
        case StationClock.OverhastySecondHand:
          context.rotate(Math.min((seconds + millis) * (60.0 / 58.5), 60.0) * Math.PI / 30);
          break;
        case StationClock.CreepingSecondHand:
          context.rotate((seconds + millis) * Math.PI / 30);
          break;
        case StationClock.BouncingSecondHand:
          context.rotate(seconds * Math.PI / 30);
          break;
        case StationClock.ElasticBouncingSecondHand:
          if (this.lastSecond != seconds) {
            this.secondHandAnimationStep = 3;
            this.lastSecond = seconds;
          }
          context.rotate((seconds + this.getAnimationOffset(this.secondHandAnimationStep)) * Math.PI / 30);
          this.secondHandAnimationStep--;
          break;
      }
      this.setShadow(context, this.handShadowColor, this.handShadowOffsetX, this.handShadowOffsetY, this.handShadowBlur);
      switch (this.secondHand) {
        case StationClock.BarSecondHand:
      	  this.fillPolygon(context, this.secondHandColor, -0.006, -0.92, 0.006, -0.92, 0.028, 0.23, -0.028, 0.23);
      	  break;
        case StationClock.HoleShapedSecondHand:
          this.fillPolygon(context, this.secondHandColor, 0.0, -0.9, 0.011, -0.889, 0.01875, -0.6, -0.01875, -0.6, -0.011, -0.889);
          this.fillPolygon(context, this.secondHandColor, 0.02, -0.4, 0.025, 0.22, -0.025, 0.22, -0.02, -0.4);
          this.strokeCircle(context, this.secondHandColor, 0, -0.5, 0.083, 0.066);
      	  break;
        case StationClock.NewHoleShapedSecondHand:
          this.fillPolygon(context, this.secondHandColor, 0.0, -0.95, 0.015, -0.935, 0.0187, -0.65, -0.0187, -0.65, -0.015, -0.935);
          this.fillPolygon(context, this.secondHandColor, 0.022, -0.45, 0.03, 0.27, -0.03, 0.27, -0.022, -0.45);
          this.strokeCircle(context, this.secondHandColor, 0, -0.55, 0.085, 0.07);
      	  break;
        case StationClock.SwissSecondHand:
      	  this.strokeLine(context, this.secondHandColor, 0.0, -0.6, 0.0, 0.35, 0.026);
      	  this.fillCircle(context, this.secondHandColor, 0, -0.64, 0.1);
          break;
        case StationClock.ViennaSecondHand:
          this.strokeLine(context, this.secondHandColor, 0.0, -0.6, 0.0, 0.35, 0.026);
          this.fillCircle(context, this.secondHandColor, 0, -0.64, 0.1);
          break;
      }
      context.restore();
      
      // draw clock boss
      if (this.boss != StationClock.NoBoss) {
        context.save();
        this.setShadow(context, this.bossShadowColor, this.bossShadowOffsetX, this.bossShadowOffsetY, this.bossShadowBlur);
        switch (this.boss) {
    		  case StationClock.BlackBoss:
    		    this.fillCircle(context, 'rgb(0,0,0)', 0, 0, 0.1);
    		    break;
    		  case StationClock.RedBoss:
    		    this.fillCircle(context, 'rgb(220,0,0)', 0, 0, 0.06);
    		    break;
          case StationClock.ViennaBoss:
            this.fillCircle(context, 'rgb(0,0,0)', 0, 0, 0.07);
            break;
        }
        context.restore();
      }
      
      context.restore();
    }
  }
};

StationClock.prototype.getAnimationOffset = function(animationStep) {
  switch (animationStep) {
    case 3: return  0.2;
    case 2: return -0.1;
    case 1: return  0.05;
  }
  return 0;
};

StationClock.prototype.setShadow = function(context, color, offsetX, offsetY, blur) {
  if (color) {
  	context.shadowColor   = color;
  	context.shadowOffsetX = this.radius * offsetX;
  	context.shadowOffsetY = this.radius * offsetY;
  	context.shadowBlur    = this.radius * blur;
  }
};

StationClock.prototype.fillCircle = function(context, color, x, y, radius) {
  if (color) {
    context.beginPath();
    context.fillStyle = color;
    context.arc(x * this.radius, y * this.radius, radius * this.radius, 0, 2 * Math.PI, true);
    context.fill();
  }
};

StationClock.prototype.strokeCircle = function(context, color, x, y, radius, lineWidth) {
  if (color) {
    context.beginPath();
    context.strokeStyle = color;
    context.lineWidth = lineWidth * this.radius;
    context.arc(x * this.radius, y * this.radius, radius * this.radius, 0, 2 * Math.PI, true);
    context.stroke();
  }
};

StationClock.prototype.fillSquare = function(context, color, x, y, size) {
  if (color) {
    context.fillStyle = color;
    context.fillRect((x - size / 2) * this.radius, (y -size / 2) * this.radius, size * this.radius, size * this.radius);
  }
};

StationClock.prototype.strokeSquare = function(context, color, x, y, size, lineWidth) {
  if (color) {
    context.strokeStyle = color;
    context.lineWidth = lineWidth * this.radius;
    context.strokeRect((x - size / 2) * this.radius, (y -size / 2) * this.radius, size * this.radius, size * this.radius);
  }
};

StationClock.prototype.strokeLine = function(context, color, x1, y1, x2, y2, width) {
  if (color) {
	  context.beginPath();
	  context.strokeStyle = color;
	  context.moveTo(x1 * this.radius, y1 * this.radius);
	  context.lineTo(x2 * this.radius, y2 * this.radius);
	  context.lineWidth = width * this.radius;
	  context.stroke();
  }
};

StationClock.prototype.fillPolygon = function(context, color, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5) {
  if (color) {
	  context.beginPath();
	  context.fillStyle = color;
	  context.moveTo(x1 * this.radius, y1 * this.radius);
	  context.lineTo(x2 * this.radius, y2 * this.radius);
	  context.lineTo(x3 * this.radius, y3 * this.radius);
	  context.lineTo(x4 * this.radius, y4 * this.radius);
	  if ((x5 != undefined) && (y5 != undefined)) {
	    context.lineTo(x5 * this.radius, y5 * this.radius);
	  }
	  context.lineTo(x1 * this.radius, y1 * this.radius);
	  context.fill();
  }
};

StationClock.prototype.fillSymmetricPolygon = function(context, color, points) {
    context.beginPath();
    context.fillStyle = color;
    context.moveTo(points[0][0] * this.radius, points[0][1] * this.radius);
    for (var i = 1; i < points.length; i++) {
      context.lineTo(points[i][0] * this.radius, points[i][1] * this.radius);
    }
    for (var i = points.length - 1; i >= 0; i--) {
      context.lineTo(0 - points[i][0] * this.radius, points[i][1] * this.radius);
    }
    context.lineTo(points[0][0] * this.radius, points[0][1] * this.radius);
    context.fill();
};

StationClock.prototype.strokeSymmetricPolygon = function(context, color, points, width) {
    context.beginPath();
    context.strokeStyle = color;
    context.moveTo(points[0][0] * this.radius, points[0][1] * this.radius);
    for (var i = 1; i < points.length; i++) {
      context.lineTo(points[i][0] * this.radius, points[i][1] * this.radius);
    }
    for (var i = points.length - 1; i >= 0; i--) {
      context.lineTo(0 - points[i][0] * this.radius, points[i][1] * this.radius);
    }
    context.lineTo(points[0][0] * this.radius, points[0][1] * this.radius);
    context.lineWidth = width * this.radius;
    context.stroke();
};


