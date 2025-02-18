
type pw_auth.pug | pug --pretty -p . -O "{type: 'register'}" > pw_auth_register.xml
type pw_auth.pug | pug --pretty -p . -O "{type: 'login'}" > pw_auth_login.xml
rem Finished generating XMLs