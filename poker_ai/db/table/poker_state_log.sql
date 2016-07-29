CREATE TABLE poker_state_log
(
	state_id                  NUMBER(38, 0),
	tournament_id             NUMBER(10, 0),
	tournament_mode           NUMBER(1, 0),
	evolution_trial_id        NUMBER(10, 0),
	player_count              NUMBER(2, 0),
	buy_in_amount             NUMBER(10, 0),
	tournament_in_progress    NUMBER(1, 0),
	current_game_number       NUMBER(10, 0),
	game_in_progress          NUMBER(1, 0),
	small_blind_seat_number   NUMBER(2, 0),
    big_blind_seat_number     NUMBER(2, 0),
    turn_seat_number          NUMBER(2, 0),
    small_blind_value         NUMBER(10, 0),
    big_blind_value           NUMBER(10, 0),
    betting_round_number      NUMBER(1, 0),
	betting_round_in_progress NUMBER(1, 0),
	last_to_raise_seat_number NUMBER(2, 0),
	min_raise_amount          NUMBER(10, 0),
    community_card_1          NUMBER(2, 0),
    community_card_2          NUMBER(2, 0),
    community_card_3          NUMBER(2, 0),
    community_card_4          NUMBER(2, 0),
    community_card_5          NUMBER(2, 0)
) INMEMORY;

ALTER TABLE poker_state_log ADD (
	CONSTRAINT psl_pk_sid PRIMARY KEY (state_id)
);
