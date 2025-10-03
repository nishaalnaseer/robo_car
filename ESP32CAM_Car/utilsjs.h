#ifndef UTILSJS_H
#define UTILSJS_H

#include <pgmspace.h>

const char utils_js[] PROGMEM = R"rawliteral(
export class Joystick {
  constructor(canvasId, options = {}) {
    this.canvas = document.getElementById(canvasId);
    this.ctx = this.canvas.getContext('2d');

    // Options
    this.options = {
      outerRadius: options.outerRadius || 120,
      innerRadius: options.innerRadius || 30,
      outerColor: options.outerColor || 'rgba(161,43,43,0)',
      innerColor: options.innerColor || '#a42727',
      activeColor: options.activeColor || '#00ff00',
      ...options
    };

    // State
    this.centerX = this.canvas.width / 2;
    this.centerY = this.canvas.height / 2;
    this.knobX = this.centerX;
    this.knobY = this.centerY;
    this.isDragging = false;
    this.maxDistance = this.options.outerRadius - this.options.innerRadius;

    // Callbacks
    this.onMove = options.onMove || (() => {});
    this.onStart = options.onStart || (() => {});
    this.onEnd = options.onEnd || (() => {});

    this.init();
  }

  init() {
    this.draw();
    this.addEventListeners();
  }

  addEventListeners() {
    // Mouse events
    this.canvas.addEventListener('mousedown', this.handleStart.bind(this));
    this.canvas.addEventListener('mousemove', this.handleMove.bind(this));
    this.canvas.addEventListener('mouseup', this.handleEnd.bind(this));

    // Global mouse events for when dragging outside canvas
    document.addEventListener('mousemove', this.handleGlobalMove.bind(this));
    document.addEventListener('mouseup', this.handleGlobalEnd.bind(this));

    // Touch events for mobile
    this.canvas.addEventListener('touchstart', this.handleStart.bind(this));
    this.canvas.addEventListener('touchmove', this.handleMove.bind(this));
    this.canvas.addEventListener('touchend', this.handleEnd.bind(this));
    this.canvas.addEventListener('touchcancel', this.handleEnd.bind(this));

    // Prevent context menu on right click
    this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
  }

  handleStart(e) {
    e.preventDefault();
    const pos = this.getEventPos(e);
    const distance = this.getDistance(pos.x, pos.y);

    if (distance <= this.options.outerRadius) {
      this.isDragging = true;
      this.updateKnobPosition(pos.x, pos.y);
      this.onStart(this.getValues());
    }
  }

  handleMove(e) {
    if (!this.isDragging) return;

    e.preventDefault();
    const pos = this.getEventPos(e);
    this.updateKnobPosition(pos.x, pos.y);
    this.onMove(this.getValues());
  }

  handleEnd(e) {
    if (!this.isDragging) return;

    e.preventDefault();
    this.isDragging = false;

    // Return to center with animation
    this.animateReturn();
    this.onEnd(this.getValues());
  }

  handleGlobalMove(e) {
    if (!this.isDragging) return;

    e.preventDefault();
    const pos = this.getGlobalEventPos(e);
    this.updateKnobPosition(pos.x, pos.y);
    this.onMove(this.getValues());
  }

  handleGlobalEnd(e) {
    if (!this.isDragging) return;

    e.preventDefault();
    this.isDragging = false;

    // Return to center with animation
    this.animateReturn();
    this.onEnd(this.getValues());
  }

  getGlobalEventPos(e) {
    const rect = this.canvas.getBoundingClientRect();
    return {
      x: e.clientX - rect.left,
      y: e.clientY - rect.top
    };
  }

  getEventPos(e) {
    const rect = this.canvas.getBoundingClientRect();
    const clientX = e.clientX || (e.touches && e.touches[0].clientX);
    const clientY = e.clientY || (e.touches && e.touches[0].clientY);

    return {
      x: clientX - rect.left,
      y: clientY - rect.top
    };
  }

  getDistance(x, y) {
    const dx = x - this.centerX;
    const dy = y - this.centerY;
    return Math.sqrt(dx * dx + dy * dy);
  }

  updateKnobPosition(x, y) {
    const dx = x - this.centerX;
    const dy = y - this.centerY;
    const distance = Math.sqrt(dx * dx + dy * dy);

    if (distance <= this.maxDistance) {
      this.knobX = x;
      this.knobY = y;
    } else {
      const angle = Math.atan2(dy, dx);
      this.knobX = this.centerX + Math.cos(angle) * this.maxDistance;
      this.knobY = this.centerY + Math.sin(angle) * this.maxDistance;
    }

    this.draw();
  }

  animateReturn() {
    const startX = this.knobX;
    const startY = this.knobY;
    const startTime = Date.now();
    const duration = 200; // ms

    const animate = () => {
      const elapsed = Date.now() - startTime;
      const progress = Math.min(elapsed / duration, 1);

      // Easing function
      const easeOut = 1 - Math.pow(1 - progress, 3);

      this.knobX = startX + (this.centerX - startX) * easeOut;
      this.knobY = startY + (this.centerY - startY) * easeOut;

      this.draw();

      if (progress < 1) {
        requestAnimationFrame(animate);
      }
    };

    animate();
  }

  draw() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

    // Draw outer circle (base)
    this.ctx.beginPath();
    this.ctx.arc(this.centerX, this.centerY, this.options.outerRadius, 0, 2 * Math.PI);
    this.ctx.fillStyle = this.options.outerColor;
    this.ctx.fill();
    this.ctx.strokeStyle = 'rgba(161,43,43,0)';
    this.ctx.lineWidth = 3;
    this.ctx.stroke();

    // Draw center cross
    this.ctx.beginPath();
    this.ctx.strokeStyle = 'rgba(161,43,43,0)';
    this.ctx.lineWidth = 1;
    // Horizontal line
    this.ctx.moveTo(this.centerX - 20, this.centerY);
    this.ctx.lineTo(this.centerX + 20, this.centerY);
    // Vertical line
    this.ctx.moveTo(this.centerX, this.centerY - 20);
    this.ctx.lineTo(this.centerX, this.centerY + 20);
    this.ctx.stroke();

    // Draw knob (inner circle)
    this.ctx.beginPath();
    this.ctx.arc(this.knobX, this.knobY, this.options.innerRadius, 0, 2 * Math.PI);
    this.ctx.fillStyle = this.isDragging ? this.options.activeColor : this.options.innerColor;
    this.ctx.fill();
    this.ctx.strokeStyle = this.isDragging ? '#ffffff' : '#999999';
    this.ctx.lineWidth = 2;
    this.ctx.stroke();

    // Draw connecting line
    if (this.isDragging || (this.knobX !== this.centerX || this.knobY !== this.centerY)) {
      this.ctx.beginPath();
      this.ctx.moveTo(this.centerX, this.centerY);
      this.ctx.lineTo(this.knobX, this.knobY);
      this.ctx.strokeStyle = this.isDragging ? this.options.activeColor : 'rgba(161,43,43,0)';
      this.ctx.lineWidth = 2;
      this.ctx.stroke();
    }
  }

  getValues() {
    const dx = this.knobX - this.centerX;
    const dy = this.knobY - this.centerY;
    const distance = Math.sqrt(dx * dx + dy * dy);
    const maxDistance = this.maxDistance;

    return {
      x: Math.round((dx / maxDistance) * 100) / 100, // -1 to 1
      y: Math.round((-dy / maxDistance) * 100) / 100, // -1 to 1 (inverted Y)
      distance: Math.round((distance / maxDistance) * 100) / 100, // 0 to 1
      angle: Math.round((Math.atan2(-dy, dx) * 180 / Math.PI + 360) % 360), // 0 to 359
      raw: { x: dx, y: dy, distance }
    };
  }
}

export class CircleMagik {
  constructor() {
    this.radius = 1;
    this.xOrigin = 0;
    this.yOrigin = 0;
  }

  static calcDistance(x1, y1, x2, y2) {
    /*
    Calculate distance between two coordinates;
     */
    const val = Math.pow((y1 - y2), 2) + Math.pow((x1 - x2), 2);
    return Math.sqrt(val)
  }

  isInsideCircle(xCor, yCor) {
    /*
    returns true if coordinates are inside the circle
     */
    const distance = CircleMagik.calcDistance(xCor, yCor, this.xOrigin, this.yOrigin);
    return distance > this.radius;
  }

  calculateGradient(xCor, yCor) {
    return ((yCor - this.yOrigin) / (xCor - this.xOrigin))
  }

  applyMagic(xCor, yCor) {
    /*
    if coordinates are outside the circle it calculates the nearest
    points on the circumference and returns those values as x and y
    if coordinates are inside the circle it returns coordinates as is
    well this does assume radius is at 0,0 even if class constructor says otherwise

     */
    if(!this.isInsideCircle(xCor, yCor)) {
      return {xValue: xCor, yValue: yCor}
    }

    const gradient = this.calculateGradient(xCor, yCor);

    // y = mx + c, c=0
    // y = mx, using this equation we gonna calculate
    // where it intersects circle

    // y^2 + x^2 = this.radius, equation for circle
    // (gradient * x)^2 + x^2 = this.radius, we gonna say x^2 part as 1, and (gradient * x)^2 as gradient^2
    // this leaves leftHand as (gradient ^ 2) +1

    const leftHand = Math.pow(gradient, 2) +1;
    const xSquare = this.radius/leftHand;
    const sqrt = Math.sqrt(xSquare);

    // now we have two values for x, + sqrt and - sqrt
    const x1 = sqrt;
    const x2 = sqrt * -1;
    const y1 = gradient * x1;
    const y2 = gradient * x2;

    const dist1  = CircleMagik.calcDistance(x1, y1, xCor, yCor);
    const dist2  = CircleMagik.calcDistance(x2, y2, xCor, yCor);
    if(dist1 < dist2) {
      return {xValue: x1, yValue: y1};
    }
    return {xValue: x2, yValue: y2};
  }
}
)rawliteral";

#endif