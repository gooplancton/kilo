vim_mode will be 0 for normal and 1 for insert
['vim_is_normal [vim_mode 0 eq] define]
['vim_is_insert [vim_mode 1 eq] define] 

start in normal mode by default
['vim_mode 0 define]

i enters insert mode
['kilo_onkey 105 ['vim_mode 1 define]]

escape returns to normal mode
['kilo_onkey 27 ['vim_mode 0 define]]

Basic movement with hjkl (no repeaters for now)

['vim_h [
	vim_is_normal
	[1000 kilo_process_key] 	move left in normal mode
	[104 kilo_process_key] 		normal execution in insert mode 
	ifelse
] define]
['kilo_onkey 104 'vim_h]

['vim_j [
	vim_is_normal
	[1002 kilo_process_key] 	move right in normal mode
	[106 kilo_process_key] 		normal execution in insert mode 
	ifelse
] define]
['kilo_onkey 106 'vim_j]

['vim_k [
	vim_is_normal
	[1003 kilo_process_key] 	move right in normal mode
	[107 kilo_process_key] 		normal execution in insert mode 
	ifelse
] define]
['kilo_onkey 107 'vim_j]

['vim_l [
	vim_is_normal
	[1001 kilo_process_key] 	move right in normal mode
	[108 kilo_process_key] 		normal execution in insert mode 
	ifelse
] define]
['kilo_onkey 108 'vim_l]
