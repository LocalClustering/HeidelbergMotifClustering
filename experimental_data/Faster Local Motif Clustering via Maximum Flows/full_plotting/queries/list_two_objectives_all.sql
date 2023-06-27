/* List two given objectives for all instances */
SELECT	generic_objective2 AS ind,
	generic_objective1 AS "generic_label"
FROM	generic_table
WHERE	algorithm = "generic_algorithm" 
ORDER BY generic_objective2, generic_objective1 ASC;
