﻿using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MyWPFApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_ClickAdd(object sender, RoutedEventArgs e)
        {
            lbResult.Content = MyDLL.Add(int.Parse(tbInput_A.Text), int.Parse(tbInput_B.Text)).ToString();
        }

        private void Button_ClickSub(object sender, RoutedEventArgs e)
        {
            lbResult.Content = MyDLL.Subtract(int.Parse(tbInput_A.Text), int.Parse(tbInput_B.Text)).ToString();
        }
        private void Button_ClickMul(object sender, RoutedEventArgs e)
        {
            lbResult.Content = MyDLL.Multiply(int.Parse(tbInput_A.Text), int.Parse(tbInput_B.Text)).ToString();
        }
        private void Button_ClickDiv(object sender, RoutedEventArgs e)
        {
            lbResult.Content = MyDLL.Divide(int.Parse(tbInput_A.Text), int.Parse(tbInput_B.Text)).ToString();
        }
    }
}