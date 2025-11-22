vim_mode will be 0 for normal and 1 for insert
[[vim_mode 0 eq] 'vim_is_normal define]
[[vim_mode 1 eq] 'vim_is_insert define] 

start in normal mode by default
[0 'vim_mode define]

i enters insert mode if the editor is in normal mode
[[
	vim_is_normal
	['vim_mode 1 define]
	[105 kilo_process_key]
	ifelse
] 'vim_i define]
[105 'vim_i kilo_onkey]

escape returns to normal mode
[27 [0 'vim_mode define 27 kilo_process_key] kilo_onkey]

Basic movement with hjkl (no repeaters for now)

['vim_h [
	vim_is_normal
	[1000 kilo_process_key]
	[104 kilo_process_key]
	ifelse
] define]
[104 'vim_h kilo_onkey]

[[
	vim_is_normal
	[1003 kilo_process_key]
	[106 kilo_process_key]
	ifelse
] 'vim_j define]
[106 'vim_j kilo_onkey]

[[
	vim_is_normal
	[1002 kilo_process_key]
	[107 kilo_process_key] 
	ifelse
] 'vim_k define]
[107 'vim_k kilo_onkey]

[[
	vim_is_normal
	[1001 kilo_process_key]
	[108 kilo_process_key]
	ifelse
] 'vim_l define]
[108 'vim_l kilo_onkey]
