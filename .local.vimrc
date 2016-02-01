" Syntastic suggested values for status line and operation
set statusline+=%#warningmsg#
set statusline+=%{SyntasticStatuslineFlag()}
set statusline+=%*
let g:always_populate_loc_list = 1
let g:syntastic_always_populate_loc_list = 1
let g:syntastic_auto_loc_list = 1
let g:syntastic_check_on_open = 1
let g:syntastic_check_on_wq = 0

" What compiler to use and the config file
let g:syntastic_c_compiler='x86_64-elf-gcc'
let g:syntastic_c_config_file='.syntastic_c_config'

" Compiler options
let g:syntatic_c_compiler_options=['-ffreestanding',  '-nostdlib', '-nostartfiles']
let g:syntatic_c_compiler_options+=['-nodefaultlibs', '-mno-red-zone']
let g:syntatic_c_compiler_options+=['-pipe', '-Wall', '-Winvalid-pch', '-std=c11']
let g:syntatic_c_compiler_options+=['-O2', '-g', '-Dpc_x86_64', '-Dx86']
let g:syntatic_c_compiler_options+=['m64', '-mno-red-zone']
