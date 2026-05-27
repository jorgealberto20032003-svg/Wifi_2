<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use App\Models\PersonalMedico;

class PersonalMedicoSeeder extends Seeder
{
    public function run(): void
    {
        PersonalMedico::create(['nombre' => 'Fernando Robles', 'num_clinica' => 12]);
        PersonalMedico::create(['nombre' => 'Jorge Casas', 'num_clinica' => 5]);
        PersonalMedico::create(['nombre' => 'Dr. Armenta', 'num_clinica' => 24]);
    }
}