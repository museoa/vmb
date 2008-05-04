#set xtics border ("50" 50,"100" 100,"500" 500,"1000" 1000,"5000" 5000,"10000" 10000)
set output "perf.png"
set terminal png medium size 640,480
plot 'cache.dat' with lines, 'nocache.dat' with lines
