function! Setup()
  execute "cd C:/Users/gauta/Codes/motorways"
  execute "Tnew"
  execute "T shell.bat"
  execute "vnew"
endfunction

map <leader>s :call Setup()<cr>

function! Build()
  execute "b1"
  execute "T cls"
  execute "T build.bat"
endfunction

map <leader>b :call Build()<cr>
