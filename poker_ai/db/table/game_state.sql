CREATE GLOBAL TEMPORARY TABLE game_state
(
	small_blind_seat_number   NUMBER(2, 0),
    big_blind_seat_number     NUMBER(2, 0),
    turn_seat_number          NUMBER(2, 0),
    small_blind_value         NUMBER(10, 0),
    big_blind_value           NUMBER(10, 0),
    betting_round_number      NUMBER(1, 0),
	betting_round_in_progress VARCHAR2(1),
	last_to_raise_seat_number NUMBER(2, 0),
	min_raise_amount          NUMBER(10, 0),
    community_card_1          NUMBER(2, 0),
    community_card_2          NUMBER(2, 0),
    community_card_3          NUMBER(2, 0),
    community_card_4          NUMBER(2, 0),
    community_card_5          NUMBER(2, 0)
) ON COMMIT PRESERVE ROWS;
