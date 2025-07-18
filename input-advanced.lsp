3
(define fib (lambda (n) 
			  (if (eq n 0) 
				0 
				(if (eq n 1) 
				  1 
				  (+ (fib (- n 2)) (fib (- n 1)))))))
(define map (lambda (f lst) 
			  (if (is-nil? lst)
				()
				(cons (f (car lst)) (map f (cdr lst))))))
(map (lambda (n) (fib (+ n 1))) (cons 0 (cons 1 (cons 2 (cons 3 (cons 4 (cons 5 (cons 6 (cons 7 (cons 8 (cons 9 ())))))))))))
