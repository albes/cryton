<?xml version="1.0" encoding="UTF-8" standalone="no"?>

<svg
   version="1.1"
   id="svg1"
   width="512"
   height="512"
   viewBox="0 0 512 512"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <defs
     id="defs1" />
  <g
     id="g1">
    <path
       id="path1"
       d="M 376.242,192 C 376.242,293.754 293.754,376.242 192,376.242 90.246,376.242 7.758,293.754 7.758,192 7.758,90.246 90.246,7.758 192,7.758 293.754,7.758 376.242,90.246 376.242,192 Z"
       style="fill:#000000;fill-opacity:1;fill-rule:nonzero;stroke:none"
       transform="scale(1.3333333)" />
    <path
       id="path2"
       d="m 196.645,136.199 -60.649,-34.097 57.984,-35.661 60.645,34.094 z"
       style="fill:#9955ff;fill-opacity:1;fill-rule:nonzero;stroke:none"
       transform="scale(1.3333333)" />
    <path
       id="path3"
       d="M 168.211,105.797 H 222.41 V 157.34 H 168.211 Z"
       style="fill:#9955ff;fill-opacity:1;fill-rule:nonzero;stroke:none"
       transform="scale(1.3333333)" />
    <path
       id="path4"
       d="M 104.621,71.672 C 62.781,98.426 37.465,144.66 37.465,194.32 c 0,38.61 15.336,75.637 42.637,102.938 27.3,27.301 64.328,42.637 102.937,42.637 40.84,0 79.801,-17.153 107.375,-47.278 -23.414,14.973 -50.625,22.93 -78.418,22.93 -80.398,-0.004 -145.574,-65.176 -145.574,-145.574 0,-36.391 13.629,-71.457 38.199,-98.301 z"
       style="fill:#ffe680;fill-opacity:1;fill-rule:nonzero;stroke:none"
       transform="scale(1.3333333)" />
    <path
       id="path5"
       d="m 147.422,120.168 c -0.363,-0.012 -0.66,0.035 -0.887,0.144 -3.949,1.922 -6.988,33.954 -5.687,48.094 -0.008,0.25 -0.012,0.5 -0.012,0.75 0,18.942 16.633,35.516 40.543,40.399 -5.723,10.5 -9.637,26.902 -10.848,45.453 -8.429,7.695 -13.281,18.922 -13.281,30.738 0,6.922 1.668,13.731 4.848,19.774 4.367,1.566 8.812,2.925 13.308,4.07 0.004,0.015 0.008,0.031 0.012,0.047 0.137,0.027 0.273,0.054 0.41,0.082 4.926,1.261 9.91,2.261 14.942,3 0.203,1.066 0.609,2.191 1.265,3.363 3.07,5.457 11.68,11.863 14.715,18.035 3.035,6.172 0.5,12.11 -4.371,15.414 -4.871,3.301 -12.078,3.969 -14.113,7.641 -2.036,3.668 1.101,10.34 6.507,10.207 5.407,-0.133 13.079,-7.07 17.614,-14.578 4.539,-7.508 5.941,-15.582 3.804,-21.352 -2.132,-5.773 -7.808,-9.242 -11.027,-13.387 -1.031,-1.332 -1.812,-2.73 -2.441,-4.109 2.984,0.184 5.976,0.277 8.972,0.277 0.844,0 1.688,-0.007 2.532,-0.023 0.003,-0.023 0.011,-0.051 0.019,-0.074 2.84,-0.024 5.68,-0.133 8.512,-0.324 6.812,-7.543 10.617,-17.602 10.617,-28.063 0,-11.598 -4.668,-22.633 -12.828,-30.32 -1.18,-18.731 -5.113,-35.317 -10.887,-45.895 23.852,-4.914 40.422,-21.469 40.422,-40.375 0,-0.914 -0.039,-1.824 -0.113,-2.734 0.742,-15.149 -2.168,-43.559 -5.879,-45.363 -2.328,-1.129 -12.676,4.449 -22.895,11.214 -7.918,-3.234 -16.757,-4.929 -25.734,-4.929 -8.781,0 -17.434,1.621 -25.223,4.722 -9.492,-6.351 -19.285,-11.796 -22.816,-11.898 z"
       style="fill:#e3dedb;fill-opacity:1;fill-rule:nonzero;stroke:#000000;stroke-width:2.25;stroke-linecap:butt;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1"
       transform="scale(1.3333333)" />
  </g>
</svg>

![logo](https://github.com/user-attachments/assets/7a9f3508-8288-47ca-9696-b8ad3c37558d)

# Cryton

Cryton is simple interpreter for a custom, Python-like language that supports simple categories.

## Build instructions:

Make sure you have a [GCC](https://gcc.gnu.org/) installed.

If you have `make` installed on your system, just run this command from the root directory:

```shell
make
```

Otherwise run:

```shell
mkdir build
gcc bigint.c interpreter.c main.c object.c parser.c scanner.c table.c value.c -o build/cryton -lreadline
```

## Run the interpreter:

To run code in a file:

```shell
./build/cryton ./CodeExamples/Example_1.py
```

To start the interpreter in interactive mode (REPL), run:

```shell
./build/cryton
```
