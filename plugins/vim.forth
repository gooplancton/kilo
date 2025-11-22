vim_mode will be 0 for normal and 1 for insert
[[vim_mode 0 eq] 'vim_is_normal define]
[[vim_mode 1 eq] 'vim_is_insert define] 

start in normal mode by default
[0 'vim_mode define]

the motion count starts at 0 but we max it with 1 before motions
[[0 'vim_motion_count define] 'vim_reset_motion_count define]
[[1 vim_motion_count max] 'vim_get_motion_count define]
[vim_reset_motion_count]

i enters insert mode if the editor is in normal mode
[[
	vim_is_normal
	[1 'vim_mode define]
	[105 kilo_process_key]
	ifelse
] 'vim_i define]
[105 'vim_i kilo_onkey]

escape returns to normal mode
[27 [
	0 'vim_mode 		define
	0 'vim_motion_count define
	27 kilo_process_key
] kilo_onkey]

number handlers for motion count (0-9)
0 goes to the beginning of the line if vim_motion_count is already 0
[48 [
	vim_is_normal
	[
		0 vim_motion_count eq
		[kilo_get_cx [1000 kilo_process_key] times]
		[vim_motion_count 10 mul 'vim_motion_count define]
		ifelse
	]
	[48 kilo_process_key]
	ifelse
] kilo_onkey]

[49 [
	vim_is_normal
	[vim_motion_count 10 mul 1 add 'vim_motion_count define]
	[49 kilo_process_key]
	ifelse
] kilo_onkey]

[50 [
	vim_is_normal
	[vim_motion_count 10 mul 2 add 'vim_motion_count define]
	[50 kilo_process_key]
	ifelse
] kilo_onkey]

[51 [
	vim_is_normal
	[vim_motion_count 10 mul 3 add 'vim_motion_count define]
	[51 kilo_process_key]
	ifelse
] kilo_onkey]

[52 [
	vim_is_normal
	[vim_motion_count 10 mul 4 add 'vim_motion_count define]
	[52 kilo_process_key]
	ifelse
] kilo_onkey]

[53 [
	vim_is_normal
	[vim_motion_count 10 mul 5 add 'vim_motion_count define]
	[53 kilo_process_key]
	ifelse
] kilo_onkey]

[54 [
	vim_is_normal
	[vim_motion_count 10 mul 6 add 'vim_motion_count define]
	[54 kilo_process_key]
	ifelse
] kilo_onkey]

[55 [
	vim_is_normal
	[vim_motion_count 10 mul 7 add 'vim_motion_count define]
	[55 kilo_process_key]
	ifelse
] kilo_onkey]

[56 [
	vim_is_normal
	[vim_motion_count 10 mul 8 add 'vim_motion_count define]
	[56 kilo_process_key]
	ifelse
] kilo_onkey]

[57 [
	vim_is_normal
	[vim_motion_count 10 mul 9 add 'vim_motion_count define]
	[57 kilo_process_key]
	ifelse
] kilo_onkey]

motion handlers
[[
	vim_is_normal
	[vim_get_motion_count [1000 kilo_process_key] times vim_reset_motion_count]
	[104 kilo_process_key]
	ifelse
] 'vim_h define]
[104 'vim_h kilo_onkey]
[[
	vim_is_normal
	[vim_get_motion_count [1003 kilo_process_key] times vim_reset_motion_count]
	[106 kilo_process_key]
	ifelse
] 'vim_j define]
[106 'vim_j kilo_onkey]
[[
	vim_is_normal
	[vim_get_motion_count [1002 kilo_process_key] times vim_reset_motion_count]
	[107 kilo_process_key] 
	ifelse
] 'vim_k define]
[107 'vim_k kilo_onkey]
[[
	vim_is_normal
	[vim_get_motion_count [1001 kilo_process_key] times vim_reset_motion_count]
	[108 kilo_process_key]
	ifelse
] 'vim_l define]
[108 'vim_l kilo_onkey]
