package main

import (
	"fmt"
	"math"
	"sync"
)

//verifica se um numero é primo
func ehPrimo(n int) bool {
	if n <= 1 {
		return false
	}
	if n == 2 {
		return true
	}
	if n%2 == 0 {
		return false
	}
	limite := int(math.Sqrt(float64(n))) + 1
	for i := 3; i < limite; i += 2 {
		if n%i == 0 {
			return false
		}
	}
	return true
}

// Goroutine para verificar se é primo e enviar resultado ao canal
func verificadorPrimo(nums <-chan int, results chan<- int, wg *sync.WaitGroup) {
	defer wg.Done()
	for n := range nums {
		if ehPrimo(n) {
			results <- n
		}
	}
}

func main() {
	var N, M int

	// Recebe N e M como entrada
	fmt.Print("Digite o valor de N (o intervalo de números primos verificados irá de 1 até N): ")
	fmt.Scan(&N)
	fmt.Print("Digite o valor de M (quantidade de goroutines): ")
	fmt.Scan(&M)

	nums := make(chan int, N)
	results := make(chan int, N)

	var wg sync.WaitGroup

	// Inicia M goroutines
	for i := 0; i < M; i++ {
		wg.Add(1)
		go verificadorPrimo(nums, results, &wg)
	}

	// Envia números de 1 a N para o canal nums
	go func() {
		for i := 1; i <= N; i++ {
			nums <- i
		}
		close(nums)
	}()

	// Fecha o canal results após o término das goroutines
	go func() {
		wg.Wait()
		close(results)
	}()

	// Conta os números primos recebidos do canal results
	contador := 0
	for primo := range results {
		fmt.Println("Primo encontrado:", primo)
		contador++
	}

	fmt.Printf("Total de números primos encontrados: %d\n", contador)
}
