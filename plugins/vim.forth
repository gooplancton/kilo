vim_mode will be 0 for normal and 1 for insert
['vim_is_normal [vim_mode 0 eq] define]
['vim_is_insert [vim_mode 1 eq] define] 

start in normal mode by default
['vim_mode 0 define]

i enters insert mode
[105 ['vim_mode 1 define] kilo_onkey]

escape returns to normal mode
[27 ['vim_mode 0 define] kilo_onkey]

Basic movement with hjkl (no repeaters for now)

['vim_h [
	vim_is_normal
	[kilo_get_cx 1 sub kilo_set_cx]
	[104 kilo_process_key]
	ifelse
] define]
[104 'vim_h kilo_onkey]

['vim_j [
	vim_is_normal
	[kilo_get_cy 1 add kilo_set_cy]
	[106 kilo_process_key]
	ifelse
] define]
[106 'vim_j kilo_onkey]

['vim_k [
	vim_is_normal
	[kilo_get_cy 1 sub kilo_set_cy]
	[107 kilo_process_key] 
	ifelse
] define]
[107 'vim_k kilo_onkey]

['vim_l [
	vim_is_normal
	[kilo_get_cx 1 add kilo_set_cx]
	[108 kilo_process_key]
	ifelse
] define]
[108 'vim_l kilo_onkey]
