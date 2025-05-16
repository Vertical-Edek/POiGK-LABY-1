#include <vector>
#include <algorithm>
#include <functional> 
#include <memory>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include <raylib.h>
#include <raymath.h>


// --- UTILS ---
namespace Utils {
	inline static float RandomFloat(float min, float max) {
		return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
	}
}

// --- TRANSFORM, PHYSICS, LIFETIME, RENDERABLE ---
struct TransformA {
	Vector2 position{};
	float rotation{};
};

struct Physics {
	Vector2 velocity{};
	float rotationSpeed{};
};

struct Renderable {
	enum Size { SMALL = 1, MEDIUM = 2, LARGE = 4 } size = SMALL;
};

// --- RENDERER ---
class Renderer {
public:
	static Renderer& Instance() {
		static Renderer inst;
		return inst;
	}

	void Init(int w, int h, const char* title) {
		InitWindow(w, h, title);
		SetTargetFPS(60);
		screenW = w;
		screenH = h;
	}

	void Begin() {
		BeginDrawing();
		ClearBackground(BLACK);
	}

	void End() {
		EndDrawing();
	}

	void DrawPoly(const Vector2& pos, int sides, float radius, float rot) {
		DrawPolyLines(pos, sides, radius, rot, WHITE);
	}

	int Width() const {
		return screenW;
	}

	int Height() const {
		return screenH;
	}

private:
	Renderer() = default;

	int screenW{};
	int screenH{};
};

// --- ASTEROID HIERARCHY ---

class Asteroid {
public:
	Asteroid(int screenW, int screenH, Vector2 pos = Vector2(0.0f, 0.0f)) {
		init(screenW, screenH, pos);
	}
	virtual ~Asteroid() = default;

	bool Update(float dt) {
		DrawText("Zzzzz", transform.position.x, transform.position.y, 15 , RED);
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));
		transform.rotation += physics.rotationSpeed * dt;
		if (transform.position.x < -GetRadius() || transform.position.x > Renderer::Instance().Width() + GetRadius() ||
			transform.position.y < -GetRadius() || transform.position.y > Renderer::Instance().Height() + GetRadius())
			return false;
		return true;
	}
	virtual void Draw(Texture2D KOMAR_TEX, Texture2D KOMAR_DEBIL_TEX) const = 0;
	
	int GetType() const { return type; }

	Vector2 GetPosition() const {
		return transform.position;
	}

	float constexpr GetRadius() const {
		return 16.f * (float)render.size;
	}

	int GetDamage() const {
		return baseDamage * static_cast<int>(render.size);
	}

	int GetSize() const {
		return static_cast<int>(render.size);
	}

protected:
	void init(int screenW, int screenH, Vector2 pos) {
		// Choose size
		render.size = static_cast<Renderable::Size>(1 << GetRandomValue(0, 2));

		// Spawn at random edge
		switch (GetRandomValue(0, 3)) {
		case 0:
			transform.position = { Utils::RandomFloat(0, screenW), -GetRadius() };
			break;
		case 1:
			transform.position = { screenW + GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		case 2:
			transform.position = { Utils::RandomFloat(0, screenW), screenH + GetRadius() };
			break;
		default:
			transform.position = { -GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		}

		// check if spawned by dead komar and set pos
		if ( !Vector2Equals(pos, Vector2(0.0f, 0.0f)) ) {
			Vector2 DelatPos = Vector2Scale(Vector2Subtract(transform.position, pos), 0.3 );
			transform.position = Vector2Add(pos, DelatPos);
		}
		// Aim towards center with jitter
		float maxOff = fminf(screenW, screenH) * 0.1f;
		float ang = Utils::RandomFloat(0, 2 * PI);
		float rad = Utils::RandomFloat(0, maxOff);
		Vector2 center = {
										 screenW * 0.5f + cosf(ang) * rad,
										 screenH * 0.5f + sinf(ang) * rad
		};

		Vector2 dir = Vector2Normalize(Vector2Subtract(center, transform.position));
		physics.velocity = Vector2Scale(dir, Utils::RandomFloat(SPEED_MIN, SPEED_MAX));
		physics.rotationSpeed = Utils::RandomFloat(ROT_MIN, ROT_MAX);

		transform.rotation = Utils::RandomFloat(0, 360);
	}

	TransformA transform;
	Physics    physics;
	Renderable render;

	float TextSize = 5;
	int baseDamage = 0;
	static constexpr float LIFE = 10.f;
	static constexpr float SPEED_MIN = 125.f;
	static constexpr float SPEED_MAX = 250.f;
	static constexpr float ROT_MIN = 50.f;
	static constexpr float ROT_MAX = 240.f;

public:
	float size = 0.15f;
	int type = 0;
};

class TriangleAsteroid : public Asteroid {
public:
	TriangleAsteroid(int w, int h) : Asteroid(w, h) { baseDamage = 5; }
	void Draw(Texture2D KOMAR_TEX, Texture2D KOMAR_DEBIL_TEX) const override {
		Renderer::Instance().DrawPoly(transform.position, 3, GetRadius(), transform.rotation);
	}
};
class SquareAsteroid : public Asteroid {
public:
	SquareAsteroid(int w, int h) : Asteroid(w, h) { baseDamage = 10; }
	void Draw(Texture2D KOMAR_TEX, Texture2D KOMAR_DEBIL_TEX) const override {
		Renderer::Instance().DrawPoly(transform.position, 4, GetRadius(), transform.rotation);
	}
};
class PentagonAsteroid : public Asteroid {
public:
	PentagonAsteroid(int w, int h) : Asteroid(w, h) { baseDamage = 15; }
	void Draw(Texture2D KOMAR_TEX, Texture2D KOMAR_DEBIL_TEX) const override {
		Renderer::Instance().DrawPoly(transform.position, 5, GetRadius(), transform.rotation);
	}
};
class KomarDebil: public Asteroid {
public:
	KomarDebil(int w, int h, float s = 0.17f, Vector2 pos = Vector2(0.0f, 0.0f)) : Asteroid(w, h, pos) {
		baseDamage = 15;
		type = 1;
		size = s;
	}
	void Draw(Texture2D KOMAR_TEX, Texture2D KOMAR_DEBIL_TEX) const override {
		DrawTextureEx(KOMAR_DEBIL_TEX, transform.position, 0.0f, size, WHITE);
	}
};

class Komar : public Asteroid {
public:
	Komar(int w, int h, float s = 0.15f, Vector2 pos = Vector2(0.0f, 0.0f)) : Asteroid(w, h, pos) {
		baseDamage = 15;
		size = s;
	}
	void Draw(Texture2D KOMAR_TEX, Texture2D KOMAR_DEBIL_TEX) const override {
		DrawTextureEx(KOMAR_TEX, transform.position, 0.0f, size, WHITE);
	}
};

// Shape selector
enum class AsteroidShape { KOMAR = 3, KOMAR_DEBIL = 4, RANDOM = 0 };

// Factory
static inline std::unique_ptr<Asteroid> MakeAsteroid(int w, int h, AsteroidShape shape, float size = 0.20f, Vector2 pos = Vector2(0.0f, 0.0f)) {
	switch (shape) {
	case AsteroidShape::KOMAR_DEBIL:
		return std::make_unique<KomarDebil>(w, h, size, pos);
	case AsteroidShape::KOMAR:
		return std::make_unique<Komar>(w, h, size, pos);
	default: {
		return MakeAsteroid(w, h, static_cast<AsteroidShape>(3 + GetRandomValue(0, 1)));
	}
	}
}

// --- PROJECTILE HIERARCHY ---
enum class WeaponType { SZPREJ, KLAPEK, PILKA, COUNT };
class Projectile {
public:
	Projectile(Vector2 pos, Vector2 vel, int dmg, WeaponType wt)
	{
		transform.position = pos;
		physics.velocity = vel;
		baseDamage = dmg;
		type = wt;
	}
	bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));
		szprej_size += 0.9f * dt;
		if (transform.position.x < 0 ||
			transform.position.x > Renderer::Instance().Width() ||
			transform.position.y < 0 ||
			transform.position.y > Renderer::Instance().Height())
		{
			return true;
		}
		return false;
	}
	void Draw(Texture2D& KLAPEK_TEX, Texture2D& SZPREJ_TEX) const {
		if (type == WeaponType::PILKA) {
			DrawCircleV(transform.position, 5.f, BLUE);
		}
		else if(type == WeaponType::KLAPEK) {
			DrawTextureEx(KLAPEK_TEX, transform.position, 0.0f, klapek_size, WHITE);
		}
		else if (type == WeaponType::SZPREJ) {
		
			DrawTextureEx(SZPREJ_TEX, transform.position, 0.0f, szprej_size, WHITE);
		}

	}
	Vector2 GetPosition() const {
		return transform.position;
	}

	float GetRadius() const {
		return (type == WeaponType::PILKA) ? 5.f : 2.f;
	}

	int GetDamage() const {
		return baseDamage;
	}
private:
	TransformA transform;
	Physics    physics;
	int        baseDamage;
	WeaponType type;
	float szprej_size = 0.25f;
	float klapek_size = 0.1f;
};


inline static std::vector<Projectile> MakeProjectiles(
	WeaponType wt,
	const Vector2 pos,
	float speed,
	const std::vector<std::unique_ptr<Asteroid>>& asteroids)
{
	std::vector<Projectile> result;

	Vector2 vel{ 0, -speed };

	if (wt == WeaponType::KLAPEK) {
		result.emplace_back(pos, vel, 20, wt);
	}
	else if (wt == WeaponType::PILKA) {
		result.emplace_back(pos, vel, 10, wt);
	}
	else if (wt == WeaponType::SZPREJ) {
		if (!asteroids.empty()) {
			const Asteroid* closest = nullptr;
			float minDist = std::numeric_limits<float>::max();

			for (const auto& ast : asteroids) {
				float dist = Vector2Distance(pos, ast->GetPosition());
				if (dist < minDist) {
					minDist = dist;
					closest = ast.get();
				}
			}

			if (closest) {
				Vector2 dir = Vector2Normalize(Vector2Subtract(closest->GetPosition(), pos));
				vel = Vector2Scale(dir, speed);
				result.emplace_back(pos, vel, 8, wt);
				float angleOffset = 6.0f * DEG2RAD;
				float angleOffset2 = 3.0f * DEG2RAD;
				Vector2 left = Vector2Rotate(vel, -angleOffset);
				Vector2 right = Vector2Rotate(vel, angleOffset);
				Vector2 left2 = Vector2Rotate(vel, -angleOffset2);
				Vector2 right2 = Vector2Rotate(vel, angleOffset2);
				result.emplace_back(pos, left, 8, wt);
				result.emplace_back(pos, right, 8, wt);
				result.emplace_back(pos, left2, 8, wt);
				result.emplace_back(pos, right2, 8, wt);
			}
		}
		else {
			// fallback
			result.emplace_back(pos, vel, 8, wt);
		}
	}

	return result;
}


// --- SHIP HIERARCHY ---
class Ship {
public:
	Ship(int screenW, int screenH) {
		transform.position = {
			screenW * 0.5f,
			screenH * 0.5f
		};
		hp = 100;
		speed = 250.f;
		alive = true;

		// per-weapon fire rate & spacing
		fireRateKLAPEK = 5.f; // shots/sec
		fireRatePILKA = 22.f;
		spacingKLAPEK = 60.f; // px between KLAPEKs
		spacingPILKA = 20.f;
	}
	virtual ~Ship() = default;
	virtual void Update(float dt) = 0;
	virtual void Draw() const = 0;

	void TakeDamage(int dmg) {
		if (!alive) return;
		hp -= dmg;
		if (hp <= 0) alive = false;
	}

	bool IsAlive() const {
		return alive;
	}

	Vector2 GetPosition() const {
		return transform.position;
	}

	virtual float GetRadius() const = 0;

	int GetHP() const {
		return hp;
	}

	float GetFireRate(WeaponType wt) const {
		return (wt == WeaponType::KLAPEK) ? fireRateKLAPEK : fireRatePILKA;
	}

	float GetSpacing(WeaponType wt) const {
		return (wt == WeaponType::KLAPEK) ? spacingKLAPEK : spacingPILKA;
	}

protected:
	TransformA transform;
	int        hp;
	float      speed;
	bool       alive;
	float      fireRateKLAPEK;
	float      fireRatePILKA;
	float      spacingKLAPEK;
	float      spacingPILKA;
};

class PlayerShip :public Ship {
public:
	PlayerShip(int w, int h) : Ship(w, h) {
		texture = LoadTexture("butla.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2);
		scale = 0.20f;
	}
	~PlayerShip() {
		UnloadTexture(texture);
	}

	void Update(float dt) override {
		if (alive) {
			if (IsKeyDown(KEY_W)) transform.position.y -= speed * dt;
			if (IsKeyDown(KEY_S)) transform.position.y += speed * dt;
			if (IsKeyDown(KEY_A)) transform.position.x -= speed * dt;
			if (IsKeyDown(KEY_D)) transform.position.x += speed * dt;
		}
		else {
			transform.position.y += speed * dt;
		}
	}

	void Draw() const override {
		if (!alive && fmodf(GetTime(), 0.4f) > 0.2f) return;
		Vector2 dstPos = {
										 transform.position.x - (texture.width * scale) * 0.5f,
										 transform.position.y - (texture.height * scale) * 0.5f
		};
		DrawTextureEx(texture, dstPos, 0.0f, scale, WHITE);
	}

	float GetRadius() const override {
		return (texture.width * scale) * 0.3f;
	}

private:
	Texture2D texture;
	float     scale;
};

// --- APPLICATION ---
class Application {
public:
	Texture2D SZPREJ_TEX;
	Texture2D KLAPEK_TEX;
	Texture2D KOMAR_TEX;
	Texture2D KOMAR_DEBIL_TEX;

	static Application& Instance() {
		static Application inst;
		return inst;
	}

	void Run() {
		srand(static_cast<unsigned>(time(nullptr)));
		Renderer::Instance().Init(C_WIDTH, C_HEIGHT, "Asteroids OOP");

		auto player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);

		float spawnTimer = 0.f;
		float spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
		WeaponType currentWeapon = WeaponType::SZPREJ;
		float shotTimer = 0.f;

		SZPREJ_TEX = LoadTexture("spark_flame.png");
		KLAPEK_TEX = LoadTexture("crock.png");
		KOMAR_TEX = LoadTexture("komar.png");
		KOMAR_DEBIL_TEX = LoadTexture("komardebil.png");

		while (!WindowShouldClose()) {
			float dt = GetFrameTime();
			spawnTimer += dt;

			// Update player
			player->Update(dt);

			// Restart logic
			if (!player->IsAlive() && IsKeyPressed(KEY_R)) {
				player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);
				asteroids.clear();
				projectiles.clear();
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}
			// Asteroid shape switch
			if (IsKeyPressed(KEY_ONE)) {
				currentShape = AsteroidShape::KOMAR;
			}
			if (IsKeyPressed(KEY_TWO)) {
				currentShape = AsteroidShape::KOMAR_DEBIL;
			}
			if (IsKeyPressed(KEY_THREE)) {
				currentShape = AsteroidShape::RANDOM;
			}

			// Weapon switch
			if (IsKeyPressed(KEY_TAB)) {
				currentWeapon = static_cast<WeaponType>((static_cast<int>(currentWeapon) + 1) % static_cast<int>(WeaponType::COUNT));
			}

			// Shooting
			{
				if (player->IsAlive() && IsKeyDown(KEY_SPACE)) {
					shotTimer += dt;
					float interval = 1.f / player->GetFireRate(currentWeapon);
					float projSpeed = player->GetSpacing(currentWeapon) * player->GetFireRate(currentWeapon);

					while (shotTimer >= interval) {
						Vector2 p = player->GetPosition();
						p.y -= player->GetRadius();
						auto newShots = MakeProjectiles(currentWeapon, p, projSpeed, asteroids);
						projectiles.insert(projectiles.end(), newShots.begin(), newShots.end());

						shotTimer -= interval;
					}
				}
				else {
					float maxInterval = 1.f / player->GetFireRate(currentWeapon);

					if (shotTimer > maxInterval) {
						shotTimer = fmodf(shotTimer, maxInterval);
					}
				}
			}

			// Spawn asteroids
			if (spawnTimer >= spawnInterval && asteroids.size() < MAX_AST) {
				asteroids.push_back(MakeAsteroid(C_WIDTH, C_HEIGHT, currentShape));
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}

			// Update projectiles - check if in boundries and move them forward
			{
				auto projectile_to_remove = std::remove_if(projectiles.begin(), projectiles.end(),
					[dt](auto& projectile) {
						return projectile.Update(dt);
					});
				projectiles.erase(projectile_to_remove, projectiles.end());
			}

			// Projectile-Asteroid collisions O(n^2)
			for (auto pit = projectiles.begin(); pit != projectiles.end();) {
				bool removed = false;

				for (auto ait = asteroids.begin(); ait != asteroids.end(); ++ait) {
					float dist = Vector2Distance((*pit).GetPosition(), (*ait)->GetPosition());
					Vector2 ait_pos = (*ait)->GetPosition();
					if (dist < (*pit).GetRadius() + (*ait)->GetRadius()) {
						ait = asteroids.erase(ait);
						if ((*ait)->GetType() == 1) {
							asteroids.push_back(MakeAsteroid(C_WIDTH, C_HEIGHT, AsteroidShape::KOMAR, 0.1f, ait_pos));
							asteroids.push_back(MakeAsteroid(C_WIDTH, C_HEIGHT, AsteroidShape::KOMAR, 0.1f, ait_pos));
						}
						pit = projectiles.erase(pit);
						removed = true;
						break;
					}
				}
				if (!removed) {
					++pit;
				}
			}

			// Asteroid-Ship collisions
			{
				auto remove_collision =
					[&player, dt](auto& asteroid_ptr_like) -> bool {
					if (player->IsAlive()) {
						float dist = Vector2Distance(player->GetPosition(), asteroid_ptr_like->GetPosition());

						if (dist < player->GetRadius() + asteroid_ptr_like->GetRadius()) {
							player->TakeDamage(asteroid_ptr_like->GetDamage());
							return true; // Mark asteroid for removal due to collision
						}
					}
					if (!asteroid_ptr_like->Update(dt)) {
						return true;
					}
					return false; // Keep the asteroid
					};
				auto asteroid_to_remove = std::remove_if(asteroids.begin(), asteroids.end(), remove_collision);
				asteroids.erase(asteroid_to_remove, asteroids.end());
			}

			// Render everything
			{
				Renderer::Instance().Begin();

				DrawText(TextFormat("HP: %d", player->GetHP()),
					10, 10, 20, GREEN);

				const char* weaponName = "";
				switch (currentWeapon) {
				case WeaponType::KLAPEK: weaponName = "KLAPEK"; break;
				case WeaponType::PILKA: weaponName = "KAUCZUKOWA PIŁKA"; break;
				case WeaponType::SZPREJ: weaponName = "SZPREJ NA KOMARY"; break;
				default: weaponName = "UNKNOWN"; break;
				}

				DrawText(TextFormat("Weapon type: %s", weaponName),
					10, 40, 20, BLUE);

				for (const auto& projPtr : projectiles) {
					projPtr.Draw(KLAPEK_TEX, SZPREJ_TEX);
				}
				for (const auto& astPtr : asteroids) {
					astPtr->Draw(KOMAR_TEX, KOMAR_DEBIL_TEX);
				}

				player->Draw();

				Renderer::Instance().End();
			}
		}
	}
private:
	Application()
	{
		asteroids.reserve(1000);
		projectiles.reserve(10'000);
	};

	std::vector<std::unique_ptr<Asteroid>> asteroids;
	std::vector<Projectile> projectiles;

	AsteroidShape currentShape = AsteroidShape::RANDOM;

	static constexpr int C_WIDTH = 800;
	static constexpr int C_HEIGHT = 800;
	static constexpr size_t MAX_AST = 150;
	static constexpr float C_SPAWN_MIN = 0.5f;
	static constexpr float C_SPAWN_MAX = 3.0f;

	static constexpr int C_MAX_ASTEROIDS = 1000;
	static constexpr int C_MAX_PROJECTILES = 10'000;
};

int main() {
	Application::Instance().Run();
	return 0;
}