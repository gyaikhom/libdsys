set term postscript eps enhanced mono
set border 15 lw 0.2
set format x "2^{%g}"
set key left top
set xlabel "Message size (bytes)"
set ylabel "Time ({/Symbol m}-seconds)" 1.0,0.0
set size 0.7, 0.7

set output "0latency.eps"
set title "Latency"
set xrange [0:20]
plot \
'0mc.dat' using 1:3 title "dsys\\_put" with linespoints,\
'0nmc.dat' using 1:3 title "dsys\\_commit" with linespoints,\
'0blk.dat' using 1:3 title "MPI\\_Send" with linespoints,\
'0nblk.dat' using 1:3 title "MPI\\_Isend" with linespoints

set output "0latency_zoom.eps"
set title "Latency (zoom)"
set xrange [0:15]
plot \
'0mc.dat' using 1:3 title "dsys\\_put" with linespoints,\
'0nmc.dat' using 1:3 title "dsys\\_commit" with linespoints,\
'0blk.dat' using 1:3 title "MPI\\_Send" with linespoints,\
'0nblk.dat' using 1:3 title "MPI\\_Isend" with linespoints

set output "1latency.eps"
set title "Latency"
set xrange [0:20]
plot \
'1mc.dat' using 1:3 title "dsys\\_put" with linespoints,\
'1nmc.dat' using 1:3 title "dsys\\_commit" with linespoints,\
'1blk.dat' using 1:3 title "MPI\\_Send" with linespoints,\
'1nblk.dat' using 1:3 title "MPI\\_Isend" with linespoints

set output "1latency_zoom.eps"
set title "Latency (zoom)"
set xrange [0:15]
plot \
'1mc.dat' using 1:3 title "dsys\\_put" with linespoints,\
'1nmc.dat' using 1:3 title "dsys\\_commit" with linespoints,\
'1blk.dat' using 1:3 title "MPI\\_Send" with linespoints,\
'1nblk.dat' using 1:3 title "MPI\\_Isend" with linespoints

