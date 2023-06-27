/* Obtain averages per algorithm and graph */
SELECT	a.graph,
	a.algorithm, 
	ROUND(EXP(AVG(LN(generic_objective))),generic_decimal_places) AS geomean_generic_objective
FROM generic_table a
GROUP BY a.graph, a.algorithm;
