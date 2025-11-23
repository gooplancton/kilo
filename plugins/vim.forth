# vim_mode will be 0 for normal and 1 for insert
[vim_mode 0 eq] 'vim_is_normal define
[vim_mode 1 eq] 'vim_is_insert define

# start in normal mode by default
0 'vim_mode define

# the motion count starts at 0 but we max it with 1 before motions
[0 'vim_motion_count define] 'vim_reset_motion_count define
[1 vim_motion_count max] 'vim_get_motion_count define
vim_reset_motion_count

# i enters insert mode if the editor is in normal mode

105 [
	vim_is_normal
	["--INSERT--" kilo_set_status_msg 1 'vim_mode define]
	[kilo_pressed_key kilo_process_key]
	ifelse
] kilo_onkey

# escape returns to normal mode

27 [
	0 'vim_mode 		define
	0 'vim_motion_count define
	27 kilo_process_key
] kilo_onkey

# number handlers for motion count (0-9)
# 0 goes to the beginning of the line if vim_motion_count is already 0

48 [
	vim_is_normal
	[
		0 vim_motion_count eq
		[kilo_get_cx [1000 kilo_process_key] times]
		[vim_motion_count 10 mul 'vim_motion_count define]
		ifelse
	]
	[48 kilo_process_key]
	ifelse
] kilo_onkey

# other digits increase the motion count

[49 50 51 52 53 54 55 56 57]
[dup 'KEY define
	[
		vim_is_normal
		["[vim_motion_count 10 mul 48 $KEY sub add 'vim_motion_count define vim_motion_count kilo_set_status_msg]" parse_eval]
		["[$KEY kilo_process_key]" parse_eval]
		ifelse
	]
	kilo_onkey
]
foreach

# motion handlers

# h
104 [
	vim_is_normal
	[vim_get_motion_count [1000 kilo_process_key] times vim_reset_motion_count]
	[104 kilo_process_key]
	ifelse
] kilo_onkey

# j
106 [
	vim_is_normal
	[vim_get_motion_count [1003 kilo_process_key] times vim_reset_motion_count]
	[106 kilo_process_key]
	ifelse
] kilo_onkey

# k
107 [
	vim_is_normal
	[vim_get_motion_count [1002 kilo_process_key] times vim_reset_motion_count]
	[107 kilo_process_key] 
	ifelse
] kilo_onkey

# l
108 [
	vim_is_normal
	[vim_get_motion_count [1001 kilo_process_key] times vim_reset_motion_count]
	[108 kilo_process_key]
	ifelse
] kilo_onkey
