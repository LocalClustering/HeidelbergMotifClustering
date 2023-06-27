set terminal postscript enhanced color
set output '| ps2pdf - pp_chart.pdf'

set xrange [generic_xrange]
set yrange [0:1]
set datafile separator "&"

set ylabel "# instances {/Symbol \243} {/Symbol t} fastest"  font "Times,35" offset -4,0
set xlabel "{/Symbol t}" font "Times,35" offset 0,-2.5



set xtics font "Times,35" offset 0,-1
set ytics font "Times,35" offset -.5,0
set grid

set key right bottom 
set key font "Times,14"
set key samplen 4 spacing 2.6 font "Times,22" #at 1.5,0.03
set key maxrows 10 width 1

generic_xscale

#unset logscale
#unset key
plot \
