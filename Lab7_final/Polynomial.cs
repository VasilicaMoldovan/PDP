using System;
using System.Collections.Generic;
using System.Text;

namespace Lab7_final
{
    [Serializable]
    public class Polynomial
    {
        public int Degree { get; set; }
        public int[] Coefficients { get; set; }
        public int size = 0;

        public Polynomial(int s)
        {
            Degree = s;
            size = s + 1;
            Coefficients = new int[size];
        }

        public void generate_polynomial()
        {
            Random rnd = new Random();

            for (int i = 0; i < size; i++)
            {
                Coefficients[i] = rnd.Next(-10, 10);
                if (i == size - 1)
                {
                    while (Coefficients[i] == 0)
                    {
                        Coefficients[i] = rnd.Next(-10, 10);
                    }
                }
            }
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();

            for (int i = size - 1; i >= 0; i--)
            {
                if (Coefficients[i] != 0)
                {
                    if (Coefficients[i] < 0)
                    {
                        sb.Append(Coefficients[i]);
                    }
                    else if (Coefficients[i] > 0)
                    {
                        if (i < size - 1)
                        {
                            sb.Append("+");
                        }
                        sb.Append(Coefficients[i]);
                    }


                    if (i == 1)
                    {
                        sb.Append("*");
                        sb.Append("X");
                    }
                    else if (i != 0)
                    {
                        sb.Append("*");
                        sb.Append("X^");
                        sb.Append(i);
                    }

                }
            }

            return sb.ToString();
        }

        internal Polynomial add_zeros(int v)
        {
            int[] newCoef = new int[size + v];

            for (int i = 0; i < size; i++)
            {
                newCoef[i] = Coefficients[i];
            }
            for (int i = size; i < size + v; i++)
            {
                newCoef[i] = 0;
            }

            Coefficients = newCoef;
            size = Coefficients.Length;

            return this;
        }
        
    }
}
