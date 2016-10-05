=============================================================================
Numpy language notes
=============================================================================
Last updated on 10/4/16; 10/5/16
=============================================================================

*.  In order to obtain dimensions for a numpy matrix, call .shape

correct_logprobs = -np.log(probs[range(num_examples), y])
print correct_logprobs
print correct_logprobs.shape

*.  numpy.zeros_like(a, dtype=None, order='K', subok=True)

    Return an array of zeros with the same shape and type as a given array.

>>> import numpy as np
>>> x = np.arange(6)
>>> print x
[0 1 2 3 4 5]
>>> x = x.reshape((2,3))
>>> print x
[[0 1 2]
 [3 4 5]]
>>> y = np.zeros_like(x)
>>> print y
[[0 0 0]
 [0 0 0]]

*.  numpy.random.randn(d0, d1, ..., dn)

    Return a sample (or samples) from the “standard normal” distribution.

    If positive, int_like or int-convertible arguments are provided, randn
generates an array of shape (d0, d1, ..., dn), filled with random floats
sampled from a univariate “normal” (Gaussian) distribution of mean 0 and
variance 1 (if any of the d_i are floats, they are first converted to
integers by truncation). A single float randomly sampled from the
distribution is returned if no argument is provided.

*.   a = np.array([1,2,3])
a =  [1 2 3]

a2 = np.array( [ [1], [2], [3] ] )

a2 =  
[[1]
 [2]
 [3]]

*.  numpy.vstack(tup) where tup = sequence of ndarrays

    Stack arrays in sequence vertically (row wise).

    Take a sequence of arrays and stack them vertically to make a single
array.

*.  xs = []

    I = []
    I.append(10.0)
    I.append(20.0)
    I.append(30.0)
    xs.append(I)

    I2 = []
    I2.append(40.0)
    I2.append(50.0)
    I2.append(60.0)
    xs.append(I2)

    I3 = []
    I3.append(70.0)
    I3.append(80.0)
    I3.append(90.0)
    xs.append(I3)

    I4 = []
    I4.append(100.0)
    I4.append(110.0)
    I4.append(120.0)
    xs.append(I4)

    I5 = []
    I5.append(130.0)
    I5.append(140.0)
    I5.append(150.0)
    xs.append(I5)

    print "xs = ", xs
  
    xs =  [[10.0, 20.0, 30.0], [40.0, 50.0, 60.0], [70.0, 80.0, 90.0], [100.0, 110.0, 120.0], [130.0, 140.0, 150.0]]

    epx = np.vstack(xs)

    epx =  [[  10.   20.   30.]
            [  40.   50.   60.]
            [  70.   80.   90.]
            [ 100.  110.  120.]
            [ 130.  140.  150.]]

epx.T =    [[  10.   40.   70.  100.  130.]
            [  20.   50.   80.  110.  140.]
            [  30.   60.   90.  120.  150.]]

*.  numpy.outer(a,b) flattens inputs a and b if they are not already
1-dimensional

*.  numpy.ravel(a) returns a contiguous flattened array.  A 1-D array,
containing the elements of the input, is returned.

x = np.array([[1, 2, 3], [4, 5, 6]])
>>> print(np.ravel(x))
[1 2 3 4 5 6]
