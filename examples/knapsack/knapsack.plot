set term png
set output "knapsack.png"
set xrange[0:3010]
set xlabel "Number of Evaluations"
set ylabel "Fitness"
plot 'knapsack.dat' using 3:4 t 'Best Fitness' w lines, 'knapsack.dat' using 3:5 t  'Average' w lines, 'knapsack.dat' using 3:6 t 'StdDev' w lines
