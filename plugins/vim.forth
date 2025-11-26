# vim_mode will be 0 for normal and 1 for insert

[vim_mode 0 eq] 'vim_is_normal define
[vim_mode 1 eq] 'vim_is_insert define

# start in normal mode by default
0 'vim_mode define

# the motion count starts at 0 but we max it with 1 before motions
[0 'vim_motion_count define] 'vim_reset_motion_count define
[1 vim_motion_count max] 'vim_get_motion_count define
vim_reset_motion_count eval

# i enters insert mode if the editor is in normal mode
[
	vim_is_normal eval
	["--INSERT--" kilo_set_status_msg 1 'vim_mode define]
	[kilo_pressed_key kilo_process_key]
	ifelse
] 'vim_i define
105 'vim_i kilo_onkey

# escape returns to normal mode
27 [
	0 'vim_mode 		define
	0 'vim_motion_count define
	27 kilo_process_key
] kilo_onkey

# number handlers for motion count (0-9)
# 0 goes to the beginning of the line if vim_motion_count is already 0
48 [
	vim_is_normal eval
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
[dup '_key define
	$[
		vim_is_normal eval
		$[vim_motion_count 10 mul 48 ,_key sub add 'vim_motion_count define]
		$[,_key kilo_process_key]
		ifelse
	]
	'_key undefine kilo_onkey
]
foreach

# motion handlers
[
	vim_is_normal eval
	[vim_get_motion_count eval [1000 kilo_process_key] times vim_reset_motion_count eval]
	[104 kilo_process_key]
	ifelse
] 'vim_h define
104 'vim_h kilo_onkey
[
	vim_is_normal eval
	[vim_get_motion_count eval [1003 kilo_process_key] times vim_reset_motion_count eval]
	[106 kilo_process_key]
	ifelse
] 'vim_j define
106 'vim_j kilo_onkey
[
	vim_is_normal eval
	[vim_get_motion_count eval [1002 kilo_process_key] times vim_reset_motion_count eval]
	[107 kilo_process_key] 
	ifelse
] 'vim_k define
107 'vim_k kilo_onkey
[
	vim_is_normal eval
	[vim_get_motion_count eval [1001 kilo_process_key] times vim_reset_motion_count eval]
	[108 kilo_process_key]
	ifelse
] 'vim_l define
108 'vim_l kilo_onkey

# w motion - move to start of next word
[
  kilo_get_cy kilo_get_row 'row define
  
  [row kilo_get_cx at " " eq not]
  [1001 kilo_process_key]
  while
  
  [row kilo_get_cx at " " eq]
  [1001 kilo_process_key]
  while
] 'vim_w_single define

[
  vim_is_normal eval
  [vim_get_motion_count eval [vim_w_single eval] times vim_reset_motion_count eval]
  [119 kilo_process_key]
  ifelse
] 'vim_w define
119 'vim_w kilo_onkey

# b motion - move to start of previous word
[
  kilo_get_cy kilo_get_row 'row define
  
  [row kilo_get_cx at " " eq not]
  [1000 kilo_process_key]
  while
  
  [row kilo_get_cx at " " eq]
  [1000 kilo_process_key]
  while
  
] 'vim_b_single define

[
  vim_is_normal eval
  [vim_get_motion_count eval [vim_b_single eval] times vim_reset_motion_count eval]
  [98 kilo_process_key]
  ifelse
] 'vim_b define
98 'vim_b kilo_onkey
