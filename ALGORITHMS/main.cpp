#include "class.cpp"

int main(int argc, char **argv)
{
   int n = 140;
   std::unique_ptr<operate> Operate = std::make_unique<operate>(n);

   if (argc == 4)
   {
      std::string mode_str = argv[1];
      int source = std::stoi(argv[2]) - 1;
      int end = std::stoi(argv[3]) - 1;

      if (source == end || source < 0 || end < 0 || source >= n || end >= n)
      {
         std::cerr << "Invalid source and end arguments\n";
         return 0;
      }
      if (mode_str == "ksp")
      {
         auto x = Operate->get_KSP(source, end, 5);
         auto ksps = Operate->printPath(x);
         for (auto &ksp : ksps)
            std::cout << ksp << std::endl;
      }
      else if (mode_str == "maxflow")
      {
         auto x = Operate->get_maxFlow(source, end);
         auto flow = Operate->printFlow(x);
         for (const auto &x : flow)
            std::cout << x << std::endl;
      }
      else
      {
         std::cerr << "Invalid mode\n";
         return 0;
      }
   }
   else
   {
      std::cerr << "Error: Expected 3 arguments (1 string and 2 numbers)!" << std::endl;
      return 0; 
   }
   return 0;
}
