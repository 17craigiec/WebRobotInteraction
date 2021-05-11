let drag_pnt;

// Function used to send a GET request to index.js
// This GET request actually ecnodes robot functionality
function httpGetAsync(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", theUrl, true); // true for asynchronous
    xmlHttp.send(null);
}

function setup() {
  var myCanvas = createCanvas(500, 550);
  myCanvas.parent("robot_arm_div");
  w = 20;
  drag_pnt = new Draggable(250-w/2+1,350-w/2,w,w)

  document.getElementById("arm_commit").addEventListener("click", moveArm);
}

function moveArm(){
  x = drag_pnt.x+drag_pnt.w/2 - 252;
  y = 328 - drag_pnt.y+drag_pnt.h/2;
  command = "A_"+x.toString()+"_"+y.toString();
  httpGetAsync(command);
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function draw() {
  background(220);
  drawWorkspace();
  drag_pnt.over();
  drag_pnt.update();
  drag_pnt.show();
  pollBoundary();

  drawGrid();
  drawBase();
  drawArm(drag_pnt.x+drag_pnt.w/2, drag_pnt.y+drag_pnt.h/2);
}

function mousePressed() {
  drag_pnt.pressed();
}

function mouseReleased() {
  drag_pnt.released();
}

function pollBoundary() {
  // Lower bounds
  if (drag_pnt.x+drag_pnt.w/2 <= 251) {
    drag_pnt.x = 252 - drag_pnt.w/2;
  }
  if (drag_pnt.y+drag_pnt.h/2 <= 51) {
    drag_pnt.y = 52 - drag_pnt.h/2;
  }

  // Upper Bounds
  if (drag_pnt.x+drag_pnt.w/2 >= 449) {
    drag_pnt.x = 448 - drag_pnt.w/2;
  }
  if (drag_pnt.y+drag_pnt.h/2 >= 349) {
    drag_pnt.y = 348 - drag_pnt.h/2;
  }
}

function drawWorkspace() {
  push();
  translate(250, 50);
  fill(255, 0, 0, 50);
  rect(0, 0, 200, 300);
  pop();
}

function drawArm(x, y){

  x = 250 - x;
  y = y - 400;

  rot1 = 0;
  rot2 = 0;

  a = 205;
  b = 205;
  c = sqrt(pow(x,2)+pow(y,2));

  a1 = atan(y/x)
  a2 = acos( (pow(a,2)+pow(c,2)-pow(b,2))/(2*a*c) );
  a3 = acos( (pow(a,2)+pow(b,2)-pow(c,2))/(2*a*b) );

  rot1 = -(a1+a2);
  rot2 = -a3+PI+rot1;

  // Arm 1
  push();
  translate(250, 400);
  stroke(0, 0, 255, 50);
  strokeWeight(20)
  rotate(rot1);
  line(0, 0, a, 0);
  pop();

  // Arm 2
  push();
  translate(250+a*cos(rot1), 400+a*sin(rot1));
  stroke(0, 0, 255, 50);
  strokeWeight(20)
  rotate(rot2)
  line(0, 0, b, 0);
  pop();
}

function drawBase() {
  push();
  translate(200, 525);
  fill(75, 75, 75);
  ellipse(0, 0, 50, 50);
  translate(-100, 0);
  ellipse(0, 0, 50, 50);
  pop();

  push();
  translate(50, 400);
  fill(130, 130, 130);
  rect(0, 0, 200, 125, 50, 15, 10, 5);
  pop();

  push();
  translate(250, 400);
  fill(130, 130, 130);
  ellipse(0, 0, 30, 30);
  pop();
}

function drawGrid() {
	stroke(200);
	fill(120);
	for (var x=-width; x < width; x+=50) {
		line(x, -height, x, height);
		text(x, x+1, 12);
	}
	for (var y=-height; y < height; y+=50) {
		line(-width, y, width, y);
		text(y, 1, y+12);
	}
}


// =============================================================
// Robot Drive Visualization

document.onkeydown = checkKey;
document.onkeyup = checkKeyUp;
x_pos = 0;
y_pos = 100;
prev_y_pos = 100;
button_pressed = false;
delay_counter = 0;

up_pressed = false;
down_pressed = false;
left_pressed = false;
right_pressed = false;

turning_animation_counter = 0;
animation_counter = 0;
resolution = 100;

last_command = '';


function checkKey(e) {

  e = e || window.event;

  spd = 8;
  button_pressed = true;

  if (e.keyCode == '38' || up_pressed) {
    // up arrow
    up_pressed = true;
    if (y_pos == 0) {
      x_pos = 0;
    }

    y_pos += spd;

    if(y_pos > 200) {
      y_pos = 200;
    }
  }
  if (e.keyCode == '40' || down_pressed) {
    // down arrow
    down_pressed = true;
    y_pos -= 2*spd;

    if(y_pos < 0) {
      y_pos = 0;
    }
  }
  if (e.keyCode == '37' || left_pressed) {
    // left arrow
    left_pressed = true;
    if (y_pos != 0) {
      x_pos -= spd;

      if(abs(x_pos) > y_pos & x_pos < 0 & y_pos != 0) {
        x_pos = -y_pos;
      }
    }else {
      x_pos -= 1;
      if (x_pos < -1) {
        x_pos = -1;
      }
    }
  }
  if (e.keyCode == '39' || right_pressed) {
    // right arrow
    right_pressed = true;
    if (y_pos != 0) {
      x_pos += spd;

      if(x_pos > y_pos & y_pos != 0) {
        x_pos = y_pos;
      }
    }else {
      x_pos += 1;
      if (x_pos > 1) {
        x_pos = 1;
      }
    }
  }
}

function checkKeyUp(e) {
  e = e || window.event;
  if (e.keyCode == '38') {
    up_pressed = false;
  }
  else if (e.keyCode == '40') {
    down_pressed = false;
  }
  else if (e.keyCode == '37') {
    left_pressed = false;
  }
  else if (e.keyCode == '39') {
    right_pressed = false;
  }
  button_pressed = up_pressed || down_pressed || left_pressed || right_pressed;
}

var t = function( p ) {

  function drawGrid() {
    p.stroke(200);
    p.fill(120);
    for (var x=-width; x < width; x+=50) {
      p.line(x, -height, x, height-149);
      p.text(x, x+1, 12);
    }
    for (var y=-height; y < height-149; y+=50) {
      p.line(-width, y, width, y);
      p.text(y, 1, y+12);
    }
  };

  function animateMotion(x, y, w, h, start, stop){
    animation_counter++;
    if (animation_counter >= resolution) {
      animation_counter = 0;
    }

    p.push();
    p.noFill();
    p.stroke(255, 0, 0, 50);
    p.strokeWeight(20);

    delta = start-stop;
    prog = delta*animation_counter/resolution;

    if (x_pos < 0) {
      p.arc(x, y, w, h, prog, prog+0.001);
    }
    if (x_pos > 0) {
      p.arc(x, y, w, h, PI-prog, PI-prog+0.001);
    }
    if (x_pos == 0) {
      p.line(250, 250-prog, 250, 250-prog+0.001);
    }
    p.pop();
  };

  function drawArc(){
    p.push();
    p.noFill();
    p.stroke(0, 0, 255, 50);
    p.strokeWeight(20);

    if (button_pressed) {
      delay_counter = 0;
    } else {
      if (delay_counter > 5) {
        y_pos--;
        if (y_pos <= 0) {
          x_pos = 0;
        }
      }
      delay_counter++;
    }

    if (y_pos <= 0) {
      y_pos = 0;
    }

    if (y_pos != 0 & x_pos > y_pos) {
      x_pos = y_pos;
    }

    if (x_pos != 0) {
      arc_rad = (pow(y_pos, 2) + pow(x_pos, 2))/(2*x_pos);
      theta = asin(y_pos/arc_rad);

      if (x_pos < 0) {
                                                  // end theta, start theta
        arc_rad = -1*arc_rad;
        p.arc(250-arc_rad, 250, 2*arc_rad, 2*arc_rad, theta, 0);
        animateMotion(250-arc_rad, 250, 2*arc_rad, 2*arc_rad, theta, 0);
      }
      else {
                                                  // start theta, end theta
        p.arc(250+arc_rad, 250, 2*arc_rad, 2*arc_rad, PI, PI+theta);
        animateMotion(250+arc_rad, 250, 2*arc_rad, 2*arc_rad, PI, PI+theta);
      }
    }
    else {
      if (y_pos != 0) {
        p.line(250, 250, 250, 250-y_pos);
        animateMotion(1, 1, 1, 1, 250, 250-y_pos);
      }
    }

    p.pop();
  };

  function drawTurnArc() {
    p.push();
    p.noFill();
    p.stroke(0, 0, 255, 50);
    p.strokeWeight(20);

    if (turning_animation_counter > 2*PI) {
      turning_animation_counter = 0;
    }

    if (y_pos == 0 & x_pos != 0) {
      mag_dt = 10;

      if (x_pos < 0) {
        p.arc(250, 250, 200, 200, 0, -0.001);
        turning_animation_counter -= 0.02*(mag_dt*sin(turning_animation_counter)+mag_dt+0.5);
      }
      if (x_pos > 0) {
        p.arc(250, 250, 200, 200, 0, -0.001);
        turning_animation_counter += 0.02*(mag_dt*sin(turning_animation_counter)+mag_dt+0.5);
      }

      p.stroke(255, 0, 0, 50);
      p.ellipse(100*cos(turning_animation_counter)+250, 100*sin(turning_animation_counter)+250, 0, 0);
    }

    p.pop();
  }

  function drawRobot(){
    p.push();
    p.translate(200, 200);
    p.fill(130, 130, 130);
    p.rect(0, 0, 100, 125, 15, 15, 50, 50);
    p.pop();
  };

  function drawText(){
    p.push();
    var sourceText = "Use Arrow Keys To Move";
    p.textSize(32);
    p.textAlign(CENTER, CENTER);
    var middle = sourceText.length / 2;
    var left = middle - ((mouseX / width) * middle);
    var right = middle + ((mouseX / width) * middle);
    p.text(
      sourceText.substring(left, right+1),
      width/2, 475);
      p.pop();
  }

  p.setup = function() {
    p.createCanvas(500, 550);
  };

  p.draw = function() {
    p.background(220);
    drawGrid();
    drawRobot();
    drawArc();
    drawTurnArc();
    drawText();
    command = 'D_'+x_pos.toString()+'_'+y_pos.toString();
    if(command != last_command) {
      httpGetAsync(command);
    }
    last_command = command;
  };
};

var myp5 = new p5(t, 'movement_div');
