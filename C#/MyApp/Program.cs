using MyApp;

// See https://aka.ms/new-console-template for more information
Console.WriteLine("Write variable a and b: ");
string? input = Console.ReadLine();
int a = 0;
int b = 0;
if (!string.IsNullOrEmpty(input)) {
    a = int.Parse(input.Split(' ')[0]);
    if (input.Split(' ').Length > 1)
    {
        b = int.Parse(input.Split(' ')[1]);
    }
}

Console.WriteLine($"{MyDLL.Add(a, b)}");