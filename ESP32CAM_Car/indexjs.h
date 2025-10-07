#ifndef INDEXJS_H
#define INDEXJS_H

#include <pgmspace.h>

const char index_js[] PROGMEM = R"rawliteral(
import {CircleMagik, Joystick} from "./utils.js"

const streamElement = document.getElementById("stream");
let ws = null;
let lastSent = 0;
const MINIMUM_THRESHOLD = 140;
const circle = new CircleMagik();
let reconnectTimeout = null;
let reconnectDelay = 200; // can adjust or implement backoff
let gear = 4;
let lastGearChanged = 0;

// Initialize the joystick
new Joystick('joystick', {
  outerRadius: 120,
  innerRadius: 30,
  outerColor: 'rgba(26,26,26,0)',
  innerColor: 'rgba(74,74,74,0)',
  activeColor: 'rgba(122,121,121,0)',
  onMove: (values) => {
    // Handle joystick movement
    move(values);
    // You can send these values to your server/robot here
  },
  onStart: (values) => {
    move(values);
  },
  onEnd: (values) => {
    fetch("/stop");
  }
});

function setStream() {
  streamElement.innerHTML = `
    <img src="${getOrigin()}:81/stream" class="w-full h-full object-cover"
         style="rotate: 270deg" alt="stream"/>
  `;
}


function connectWebSocket() {
  if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
    return;
  }

  ws = new WebSocket(`/ws`);

  ws.onopen = () => {
    console.log("✅ Connected");
    clearTimeout(reconnectTimeout);
    reconnectTimeout = null;
    setStream();
  };

  ws.onclose = () => {
    console.log("❌ Disconnected, retrying...");
    scheduleReconnect();
  };

  ws.onerror = (err) => {
    console.log("⚠️ WebSocket error", err);
    ws.close(); // onclose will handle the reconnect
  };
}

function scheduleReconnect() {
  if (reconnectTimeout) return; // avoid duplicates
  reconnectTimeout = setTimeout(connectWebSocket, reconnectDelay);
}

connectWebSocket();

function updateDisplay(values) {
  // const values = this.getValues();
  document.getElementById('xValue').textContent = values.x.toFixed(2);
  document.getElementById('yValue').textContent = values.y.toFixed(2);
  document.getElementById('gear').textContent = gear;
}
function getOrigin() {
  return window.origin;
}

function calculateTankSteering(x, y) {
  // x: -1 (left) to 1 (right)
  // y: -1 (back) to 1 (forward)

  // Calculate raw motor values using tank steering formula
  // Forward/backward motion affects both motors equally
  // Left/right turning creates differential between motors
  let leftMotor = y + x;
  let rightMotor = y - x;

  // Optional: clamp -1 to 1
  leftMotor = Math.max(-1, Math.min(1, leftMotor));
  rightMotor = Math.max(-1, Math.min(1, rightMotor));

  let leftForward = 0;
  let leftBackward = 0;
  let rightForward = 0;
  let rightBackward = 0;

  // Scale to PWM range 140-255
  if(leftMotor > 0) {
    leftForward = MINIMUM_THRESHOLD + Math.floor(leftMotor * (255 - MINIMUM_THRESHOLD));
  } else {
    leftBackward = MINIMUM_THRESHOLD + Math.floor(-leftMotor * (255 - MINIMUM_THRESHOLD));
  }

  if(rightMotor > 0) {
    rightForward = MINIMUM_THRESHOLD + Math.floor(rightMotor * (255 - MINIMUM_THRESHOLD));
  } else {
    rightBackward = MINIMUM_THRESHOLD + Math.floor(-rightMotor * (255 - MINIMUM_THRESHOLD));
  }

  // console.log(`Motor PWM values:
  //   Left Forward: ${leftForward}, Left Backward: ${leftBackward}
  //   Right Forward: ${rightForward}, Right Backward: ${rightBackward}`);
  return {
    leftForward: rightForward,
    leftBackward: rightBackward,
    rightForward: leftForward,
    rightBackward: leftBackward,
  };
}

function move(values) {
  let led = values.led;

  // const magic = circle.applyMagic(values.x, values.y);
  // values = {x: magic.xValue, y: magic.yValue};

  updateDisplay(values);
  const timestamp = Date.now();
  if((timestamp - lastSent) < 100) {
    return;
  }
  lastSent = timestamp;

  values.y = (values.y * gear) / 4;
  values.x = (values.x * gear) / 4;

  const motors = calculateTankSteering(values.x, values.y);

  // For tank steering: left wheels move together, right wheels move together
  let leftFront = motors.leftForward;
  let leftBack = motors.leftBackward;
  let rightFront = motors.rightForward;
  let rightBack = motors.rightBackward;
  console.log(`before led ${led}`)
  led = (led ?? 0) * 255;
  console.log(`after led ${led}`)
  if(leftFront <= MINIMUM_THRESHOLD) {
    leftFront = 0;
  }
  if(leftBack <= MINIMUM_THRESHOLD) {
    leftBack = 0;
  }
  if(rightFront <= MINIMUM_THRESHOLD) {
    rightFront = 0;
  }
  if(rightBack <= MINIMUM_THRESHOLD) {
    rightBack = 0;
  }

  // Send command: timestamp,leftFront,leftBack,rightFront,rightBack
  const message = `${timestamp},${leftFront},${leftBack},${rightFront},${rightBack},${led}`;
  console.log(message);
  ws.send(message);
}

const gamepadAPI = {
  controller: {},
  turbo: false,
  connect(e) {
    console.log(e);
  },
  disconnect(e) {
    console.log(e);
  },
  update() {},
  buttonPressed() {

  },
  buttons: [],
  buttonsCache: [],
  buttonsStatus: [],
  axesStatus: [],
};

function checkPads() {
  const pads = navigator.getGamepads ? navigator.getGamepads() : [];
  const time = Date.now();
  let gamePadConnected = false;
  for (let i = 0; i < pads.length; i++) {
    const gp = pads[i]
    if (gp) {
      gamePadConnected = true;
      if(gp.buttons[4].pressed && gp.buttons[5].pressed){

      } else {
        if((time - lastGearChanged) > 500) {
          if (gp.buttons[4].pressed) {
            if(gear > 1) {
              gear--;
              lastGearChanged = time;
            }
          }
          if (gp.buttons[5].pressed) {
            if(gear < 4) {
              gear++;
              lastGearChanged = time;
            }
          }
        }
      }


      const rightX = gp.axes[2];
      const rightY = gp.axes[3];
      const rt = gp.buttons[7]; // RT is usually index 7
      let rtVal = 0;
      if (rt.pressed || rt.value > 0) {
        rtVal = rt.value.toFixed(2);
      }

      if (rightX !== undefined && rightY !== undefined) {
        move({x: rightX, y: rightY * -1, led: rtVal});
        console.log(
          `Right Stick -> X: ${rightX}, Y: ${rightY}, RT: ${rtVal}`
        );
      }
    }
  }

  const element = document.getElementById("joystick");
  if(gamePadConnected) {
    element.classList.remove("border-gray-600");
    element.classList.add("border-red-600");
  } else {
    element.classList.remove("border-red-600");
    element.classList.add("border-gray-600");
  }
}

setInterval(checkPads, 50); // poll every second

window.addEventListener("gamepadconnected", gamepadAPI.connect);
window.addEventListener("gamepaddisconnected", gamepadAPI.disconnect);
)rawliteral";

#endif