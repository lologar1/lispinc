; atoms
(define {nil true false} {} 1 0)

; functions
(define {defun} (lambda {args body} {define (head args) (lambda (tail args) body)}))
(defun {unpack fn args} {eval (join (list fn) args)})
(defun {pack f args...} {f args...})

; utilities
(defun {do args...} {if (== args... nil) {nil} {last args...}}) ; arguments evaluated in sequence (return last)
(defun {let placeholder} {((lambda {ignored} placeholder) nil)}) ; eval code within anonymous function

; list
(defun {car l} {eval (head l)}) ; return first element (unpacked) from list
(defun {len l} {if (== l nil) {0} {+ 1 (len (tail l))}}) ; recursively find length
(defun {index i l} {if (== i 0) {car l} {index (- i 1) (tail l)}}) ; get element at
(defun {last l} {index (- (len l) 1) l}) ; last element
(defun {take n l} {if (== n 0) {nil} {join (head l) (take (- n 1) (tail l))}}) ; head with quantity
(defun {pop n l} { if (== n 0) {l} {pop (- n 1) (tail l)}}) ; pop
(defun {split i l} {list (take i l) (drop i l)}) ; break into two lists (first list has i elems)
(defun {map f l} {if (== l nil) {nil} {join (list (f (car l))) (map f (tail l))}}) ; map
(defun {filter f l} {if (== l nil) {nil} {join (if (f (car l)) {head l} {nil}) (filter f (tail l))}})
(defun {foldl f acc l} {if (== l nil) {acc} {foldl f (f acc (car l)) (tail l)}})
(defun {cond cmp cases...} {if (== cases... nil) {nil} ; no more conditions: nil
  {if (== cmp (car (car cases...))) {eval (tail (car cases...))} ; matched: unpack and return
								{unpack cond (join (list cmp) (tail cases...))}}}) ; pass vaargs and retry
(define {else} true) ; obligatory

; misc (taken from book)
(defun {flip f a b} {f b a})
(defun {ghost args...} {eval args...})
(defun {comp f g x} {f (g x)})
(defun {fib n} {cond n
 {0 0}
 {1 1}
 {n (+ (fib (- n 1)) (fib (- n 2)))}})
