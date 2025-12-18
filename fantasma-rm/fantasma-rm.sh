#
#!/bin/bash

#garcia chavez erik 01275863
#universidad autonoma de baja california 
#ingenieria en computacion 
#sistemas operativos



#entonces este sera el fantasma de con bash 

GHOST_BASE="/.andromeda_shell/.ghostzone/"
GHOST_BASE_FATHER="/.adromeda_shell/"
GHOST_BASE_FINAL="/.ghostzone"

BUFFER_SIZE=1024


function_name(){
	
	#el primero parametro para el nombre del archivos 
	#el segundo para el destino si tiene se asigna si no 
	pre_DIR="${HOME}${GHOST_BASE_FATHER}"

	if [ -d "$pre_DIR" ]; then
		echo "directorio ya existe"
	else
		padre=`mkdir -p $pre_DIR`
		echo "$pre_DIR"
	fi

	cd "$pre_DIR" || exit 1

	
}

function_name










