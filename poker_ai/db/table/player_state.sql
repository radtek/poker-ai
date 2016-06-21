CREATE GLOBAL TEMPORARY TABLE player_state
(
	seat_number                   NUMBER(2, 0),
	player_id                     NUMBER(10, 0),
	current_strategy_id           NUMBER(10, 0),
	assumed_strategy_id           NUMBER(10, 0),
    hole_card_1                   NUMBER(2, 0),
    hole_card_2                   NUMBER(2, 0),
	best_hand_combination         VARCHAR2(9),
	best_hand_rank                VARCHAR2(17),
	best_hand_card_1              NUMBER(2, 0),
	best_hand_card_2              NUMBER(2, 0),
	best_hand_card_3              NUMBER(2, 0),
	best_hand_card_4              NUMBER(2, 0),
	best_hand_card_5              NUMBER(2, 0),
	hand_showing                  VARCHAR2(1),
	presented_bet_opportunity     VARCHAR2(1),
    money                         NUMBER(10, 0),
    state                         VARCHAR2(20),
	game_rank                     NUMBER(2, 0),
	tournament_rank               NUMBER(2, 0),
	games_played                  NUMBER(10, 0),
	main_pots_won                 NUMBER(10, 0),
	main_pots_split               NUMBER(10, 0),
	side_pots_won                 NUMBER(10, 0),
	side_pots_split               NUMBER(10, 0),
	average_game_profit           NUMBER(12, 2),
	flops_seen                    NUMBER(10, 0),
	turns_seen                    NUMBER(10, 0),
	rivers_seen                   NUMBER(10, 0),
	pre_flop_folds                NUMBER(10, 0),
	flop_folds                    NUMBER(10, 0),
	turn_folds                    NUMBER(10, 0),
	river_folds                   NUMBER(10, 0),
	total_folds                   NUMBER(10, 0),
	pre_flop_checks               NUMBER(10, 0),
	flop_checks                   NUMBER(10, 0),
	turn_checks                   NUMBER(10, 0),
	river_checks                  NUMBER(10, 0),
	total_checks                  NUMBER(10, 0),
	pre_flop_calls                NUMBER(10, 0),
	flop_calls                    NUMBER(10, 0),
	turn_calls                    NUMBER(10, 0),
	river_calls                   NUMBER(10, 0),
	total_calls                   NUMBER(10, 0),
	pre_flop_bets                 NUMBER(10, 0),
	flop_bets                     NUMBER(10, 0),
	turn_bets                     NUMBER(10, 0),
	river_bets                    NUMBER(10, 0),
	total_bets                    NUMBER(10, 0),
	pre_flop_total_bet_amount     NUMBER(10, 0),
	flop_total_bet_amount         NUMBER(10, 0),
	turn_total_bet_amount         NUMBER(10, 0),
	river_total_bet_amount        NUMBER(10, 0),
	total_bet_amount              NUMBER(10, 0),
	pre_flop_average_bet_amount   NUMBER(12, 2),
	flop_average_bet_amount       NUMBER(12, 2),
	turn_average_bet_amount       NUMBER(12, 2),
	river_average_bet_amount      NUMBER(12, 2),
	average_bet_amount            NUMBER(12, 2),
	pre_flop_raises               NUMBER(10, 0),
	flop_raises                   NUMBER(10, 0),
	turn_raises                   NUMBER(10, 0),
	river_raises                  NUMBER(10, 0),
	total_raises                  NUMBER(10, 0),
	pre_flop_total_raise_amount   NUMBER(10, 0),
	flop_total_raise_amount       NUMBER(10, 0),
	turn_total_raise_amount       NUMBER(10, 0),
	river_total_raise_amount      NUMBER(10, 0),
	total_raise_amount            NUMBER(10, 0),
	pre_flop_average_raise_amount NUMBER(12, 2),
	flop_average_raise_amount     NUMBER(12, 2),
	turn_average_raise_amount     NUMBER(12, 2),
	river_average_raise_amount    NUMBER(12, 2),
	average_raise_amount          NUMBER(12, 2),
	times_all_in                  NUMBER(10, 0),
	total_money_played            NUMBER(38, 0),
	total_money_won               NUMBER(38, 0) 
) ON COMMIT PRESERVE ROWS;

ALTER TABLE player_state ADD
(
	CONSTRAINT ps_pk_sn PRIMARY KEY (seat_number)
);
