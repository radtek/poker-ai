DELETE FROM evolution_trial;
DELETE FROM evolution_trial_work;
DELETE FROM player_state_log;
DELETE FROM poker_ai_log;
DELETE FROM poker_state_log;
DELETE FROM pot_contribution_log;
DELETE FROM pot_log;
DELETE FROM strategy;
DELETE FROM strategy_fitness;
DELETE FROM tournament_result;
DECLARE
	v_purge_options DBMS_AQADM.AQ$_PURGE_OPTIONS_T;
BEGIN
	DBMS_AQADM.PURGE_QUEUE_TABLE('ev_trial_work_queue_tbl', NULL, v_purge_options);
END;
COMMIT;

-- state log inspection
SELECT l.*,
       psl.current_game_number,
       psl.game_in_progress,
       psl.small_blind_value,
       psl.big_blind_value
FROM   poker_ai_log l,
       poker_state_log psl
WHERE  l.state_id = psl.state_id (+)
ORDER BY l.log_record_number; 

SELECT * FROM poker_state_log ORDER BY state_id;
SELECT * FROM player_state_log ORDER BY state_id, seat_number;
SELECT * FROM pot_log ORDER BY state_id, pot_number, betting_round_number;
SELECT * FROM pot_contribution_log ORDER BY state_id, pot_number, betting_round_number, player_seat_number;


-- money balance throughout tournament
WITH player_money AS (
	SELECT state_id,
		   SUM(money) total_player_money
	FROM   player_state_log
	GROUP BY state_id
),

pot_money AS (
	SELECT state_id,
		   SUM(pot_contribution) total_pot_money
	FROM   pot_contribution_log
	GROUP BY state_id
),

total_money AS (
	SELECT NVL(plm.state_id, pom.state_id) state_id,
		   plm.total_player_money,
		   pom.total_pot_money,
		   NVL(plm.total_player_money, 0) + NVL(pom.total_pot_money, 0) total_money
	FROM   player_money plm
		   FULL OUTER JOIN pot_money pom ON plm.state_id = pom.state_id
),

detail AS (
	SELECT tm.state_id,
		   tm.total_player_money,
		   tm.total_pot_money,
		   tm.total_money
	FROM   total_money tm,
		   poker_state_log psl
	WHERE  tm.state_id = psl.state_id
	   AND psl.game_in_progress = 1
	ORDER BY state_id DESC
)
--SELECT * FROM detail;
SELECT * FROM detail WHERE total_money != 5000;



-- monitor evolution trial
SELECT generation,
	   COUNT(*) generation_count
FROM   strategy
GROUP BY generation
ORDER BY generation;

SELECT COUNT(*) / 10 FROM tournament_result;
SELECT COUNT(*) FROM strategy_fitness;

-- analyze evolution
SELECT s.generation,
	   MIN(s.strategy_id) KEEP (DENSE_RANK FIRST ORDER BY sf.fitness_score DESC, s.strategy_id) best_strategy_id,
	   MIN(sf.fitness_score) KEEP (DENSE_RANK FIRST ORDER BY sf.fitness_score DESC, s.strategy_id) best_fitness_score,
	   MIN(sf.average_game_profit) KEEP (DENSE_RANK FIRST ORDER BY sf.fitness_score DESC, s.strategy_id) best_avgerage_game_profit,
	   ROUND(AVG(sf.fitness_score), 2) average_fitness_score,
	   ROUND(AVG(sf.average_game_profit), 2) average_average_game_profit
FROM   strategy s,
	   strategy_fitness sf
WHERE  s.strategy_id = sf.strategy_id
GROUP BY s.generation
ORDER BY generation;



------------
-- restart an interrupted trial
DECLARE
	v_evolution_trial_id evolution_trial.trial_id%TYPE := 1;
	v_purge_options      DBMS_AQADM.AQ$_PURGE_OPTIONS_T;
BEGIN

	-- purge any results generated for current generation
	EXECUTE IMMEDIATE 'TRUNCATE TABLE evolution_trial_work';
	EXECUTE IMMEDIATE 'TRUNCATE TABLE tournament_result';
	DBMS_AQADM.PURGE_QUEUE_TABLE('ev_trial_work_queue_tbl', NULL, v_purge_options);
	
	-- re-queue tournaments
	pkg_ga_evolver.enqueue_tournaments(p_trial_id => v_evolution_trial_id);

	-- start generation controller
	-- start tournament workers

END;	
