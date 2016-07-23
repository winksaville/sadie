" Syntastic suggested values for status line and operation
set statusline+=%#warningmsg#
set statusline+=%{SyntasticStatuslineFlag()}
set statusline+=%*

" Enable synstastic debugging
"let g:syntastic_debug = 255

let g:always_populate_loc_list = 1
let g:syntastic_always_populate_loc_list = 1
let g:syntastic_auto_loc_list = 1
let g:syntastic_check_on_open = 1
let g:syntastic_check_on_wq = 0

" What compiler to use and the config file
let g:syntastic_c_compiler='clang'

" Compiler options
let g:syntastic_c_compiler_options='-std=c11 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -pipe -Wall -Winvalid-pch -O2 -g -Dpc_x86_64 -mno-red-zone -DPosix=1 -Dx86=1 -DAC_PLATFORM=Posix -DAC_ARCH=x86 -m64'

" Define the syntastic config file
let g:syntastic_c_config_file='.syntastic_c_config'
