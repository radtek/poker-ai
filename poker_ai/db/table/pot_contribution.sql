CREATE TABLE pot_contribution
(
	pot_number           NUMBER(2, 0),
	betting_round_number NUMBER(1, 0),
	player_seat_number   NUMBER(2, 0),
	pot_contribution     NUMBER(10, 0)
);

ALTER TABLE pot_contribution ADD
(
	CONSTRAINT pc_pk_pnbrnpsn PRIMARY KEY (pot_number, betting_round_number, player_seat_number)
);

ALTER TABLE pot_contribution ADD
(
    CONSTRAINT pc_fk_pnbrn FOREIGN KEY (pot_number, betting_round_number) REFERENCES pot(pot_number, betting_round_number)
);
