/* Obtain averages per algorithm and graph */
SELECT	a.graph,
	a.algorithm, 
	ROUND(AVG(generic_objective),generic_decimal_places) AS avg_generic_objective
FROM generic_table a
GROUP BY a.graph, a.algorithm;
