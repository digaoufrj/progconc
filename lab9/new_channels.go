// Introduz canais na aplicação Go
package main

import (
	"fmt"
)

func tarefa(str chan string) {
	// Recebe mensagem do main e imprime
	msg := <-str
	fmt.Println("Main envia para Goroutine:", msg)
	
	// Envia resposta para o main
	str <- "Oi Main, bom dia, tudo bem?"

	msg = <-str
	fmt.Println("Main envia para Goroutine:", msg)

	str <- "Certo, entendido."

	fmt.Println("Goroutine imprime: finalizando")
}

func main() {
	// Cria um canal de comunicação não-bufferizado
	str := make(chan string)

	// Cria uma goroutine que executará a função 'tarefa'
	go tarefa(str)

	// Inicia a comunicação com a goroutine
	str <- "Olá, Goroutine, bom dia!"
	
	// Recebe resposta da goroutine e imprime
	msg := <-str
	fmt.Println("Goroutine envia para Main:", msg)

	str <- "Tudo bem! Vou terminar tá?"
	
	msg = <-str
	fmt.Println("Goroutine envia para Main:", msg)

	fmt.Println("Main imprime: finalizando")
}
