for /L %%i in (1,1,20) do (
    start cmd.exe /k "cd /d Schoenemann && null.exe datagen && exit"
)