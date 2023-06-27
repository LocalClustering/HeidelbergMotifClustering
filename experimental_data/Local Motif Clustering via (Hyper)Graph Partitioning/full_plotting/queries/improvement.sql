/* For improvement plot */
/* Obtain improvement of arithmetic means of the objective grouped by algorithm and seednode */
SELECT	row_number() over (
		ORDER BY improvement ASC
		) AS ind,
	improvement AS "generic_label"
FROM	(
	SELECT	CASE WHEN a.generic_objective = 0 THEN
			CASE WHEN c.generic_objective = 0 THEN
				0.0 
			ELSE
				2000.0
			END
		ELSE
			(100*(c.generic_objective/a.generic_objective - 1))		
		END AS improvement 
	FROM (	SELECT	b.graph,
			b.seednode, 
			AVG(b.generic_objective) AS generic_objective  
		FROM  generic_table b  
		WHERE b.algorithm="generic_algorithm" 
		GROUP BY b.graph, b.seednode) a 
	JOIN (	SELECT	b.graph,
			b.seednode, 
			AVG(b.generic_objective) generic_objective 
		FROM generic_table b 
		WHERE b.algorithm="reference_algorithm" 
		GROUP BY b.graph, b.seednode) c 
	ON	a.graph = c.graph AND
		a.seednode = c.seednode
	);
