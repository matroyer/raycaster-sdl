#include "raycaster.h"

int           init(t_raycaster *rc)
{
  rc->window = NULL;
  rc->renderer = NULL;
  rc->player_pos_x = INIT_P_POS_X;
  rc->player_pos_y = INIT_P_POS_Y;
  rc->player_dir_x = INIT_P_DIR_X;
  rc->player_dir_y = INIT_P_DIR_Y;
  rc->player_plane_x = INIT_P_PLANE_X;
  rc->player_plane_y = INIT_P_PLANE_Y;
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    fprintf(stderr,"SDL initialization failed (%s)\n",SDL_GetError());
    return (-1);
  }
  if (SDL_CreateWindowAndRenderer(WIN_X, WIN_Y, 0, &rc->window, &rc->renderer) != 0)
  {
    fprintf(stderr,"Window creation failed (%s)\n",SDL_GetError());
    return (-1);
  }
  return (0);
}

void          initial_calc(t_raycaster *rc, int x)
{
  double      camera_x;

  camera_x = 2 * x / (double)(WIN_X) - 1;
  rc->ray_dir_x = rc->player_dir_x + rc->player_plane_x * camera_x;
  rc->ray_dir_y = rc->player_dir_y + rc->player_plane_y * camera_x;
  rc->map_x = (int)(rc->player_pos_x);
  rc->map_y = (int)(rc->player_pos_y);
  rc->delta_dist_x = sqrt(1 + pow(rc->ray_dir_y, 2) / pow(rc->ray_dir_x, 2));
  rc->delta_dist_y = sqrt(1 + pow(rc->ray_dir_x, 2) / pow(rc->ray_dir_y, 2));
  if (rc->ray_dir_x < 0)
  {
    rc->step_x = -1;
    rc->side_dist_x = (rc->player_pos_x - rc->map_x) * rc->delta_dist_x;
  }
  else
  {
    rc->step_x = 1;
    rc->side_dist_x = (rc->map_x + 1.0 - rc->player_pos_x) * rc->delta_dist_x;
  }
  if (rc->ray_dir_y < 0)
  {
    rc->step_y = -1;
    rc->side_dist_y = (rc->player_pos_y - rc->map_y) * rc->delta_dist_y;
  }
  else
  {
    rc->step_y = 1;
    rc->side_dist_y = (rc->map_y + 1.0 - rc->player_pos_y) * rc->delta_dist_y;
  }
}

void          perform_dda(t_raycaster *rc)
{
  int         hit;

  hit = 0;
  while (hit == 0)
  {
    if (rc->side_dist_x < rc->side_dist_y)
    {
      rc->side_dist_x += rc->delta_dist_x;
      rc->map_x += rc->step_x;
      rc->side = 0;
    }
    else
    {
      rc->side_dist_y += rc->delta_dist_y;
      rc->map_y += rc->step_y;
      rc->side = 1;
    }
    if (worldMap[rc->map_x][rc->map_y] > 0)
      hit = 1;
  }
}

void          calc_wall_height(t_raycaster *rc)
{
  double      perp_wall_dist;
  int         line_height;

  if (rc->side == 0)
    perp_wall_dist = (rc->map_x - rc->player_pos_x + (1 - rc->step_x) / 2) / rc->ray_dir_x;
  else
    perp_wall_dist = (rc->map_y - rc->player_pos_y + (1 - rc->step_y) / 2) / rc->ray_dir_y;
  line_height = (int)(WIN_Y / perp_wall_dist);
  rc->draw_start = -line_height / 2 + WIN_Y / 2;
  if (rc->draw_start < 0)
    rc->draw_start = 0;
  rc->draw_end = line_height / 2 + WIN_Y / 2;
  if (rc->draw_end >= WIN_Y)
    rc->draw_end = WIN_Y - 1;
}

SDL_Color     convert_color(int hexa_value)
{
  SDL_Color   color;

  color.r = ((hexa_value >> 16) & 0xFF);
  color.g = ((hexa_value >> 8) & 0xFF);
  color.b = ((hexa_value) & 0xFF);
  return (color); 
}

void          draw_vert_line(t_raycaster *rc, int x)
{
  SDL_Color   color;

  if (worldMap[rc->map_x][rc->map_y] == 1)
    color = convert_color(RED);
  else if (worldMap[rc->map_x][rc->map_y] == 2)
    color = convert_color(GREEN);
  else if (worldMap[rc->map_x][rc->map_y] == 3)
    color = convert_color(BLUE);
  else if (worldMap[rc->map_x][rc->map_y] == 4)
    color = convert_color(WHITE);
  else
    color = convert_color(BLACK);
  if (rc->side == 1)
  {
    color.r /= 2;
    color.g /= 2;
    color.b /= 2;
  }
  SDL_SetRenderDrawColor(rc->renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawLine(rc->renderer, x, rc->draw_start, x, rc->draw_end);
}

void          render_frame(t_raycaster *rc)
{
  SDL_RenderPresent(rc->renderer);
  SDL_SetRenderDrawColor(rc->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(rc->renderer);
}

int           handle_events(t_raycaster *rc)
{
  SDL_Event   event;
  double      oldDirX;
  double      oldPlaneX;

  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
      return (-1);
    if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_UP)
      {
        if(worldMap[(int)(rc->player_pos_x + rc->player_dir_x * MV_SPEED)][(int)(rc->player_pos_y)] == 0) rc->player_pos_x += rc->player_dir_x * MV_SPEED;
        if(worldMap[(int)(rc->player_pos_x)][(int)(rc->player_pos_y + rc->player_dir_y * MV_SPEED)] == 0) rc->player_pos_y += rc->player_dir_y * MV_SPEED;
      }
      if (event.key.keysym.sym == SDLK_DOWN)
      {
        if(worldMap[(int)(rc->player_pos_x - rc->player_dir_x * MV_SPEED)][(int)(rc->player_pos_y)] == 0) rc->player_pos_x -= rc->player_dir_x * MV_SPEED;
        if(worldMap[(int)(rc->player_pos_x)][(int)(rc->player_pos_y - rc->player_dir_y * MV_SPEED)] == 0) rc->player_pos_y -= rc->player_dir_y * MV_SPEED;
      }
      if (event.key.keysym.sym == SDLK_RIGHT)
      {
        oldDirX = rc->player_dir_x;
        rc->player_dir_x = rc->player_dir_x * cos(-ROT_SPEED) - rc->player_dir_y * sin(-ROT_SPEED);
        rc->player_dir_y = oldDirX * sin(-ROT_SPEED) + rc->player_dir_y * cos(-ROT_SPEED);
        oldPlaneX = rc->player_plane_x;
        rc->player_plane_x = rc->player_plane_x * cos(-ROT_SPEED) - rc->player_plane_y * sin(-ROT_SPEED);
        rc->player_plane_y = oldPlaneX * sin(-ROT_SPEED) + rc->player_plane_y * cos(-ROT_SPEED);
      }
      if (event.key.keysym.sym == SDLK_LEFT)
      {
        oldDirX = rc->player_dir_x;
        rc->player_dir_x = rc->player_dir_x * cos(ROT_SPEED) - rc->player_dir_y * sin(ROT_SPEED);
        rc->player_dir_y = oldDirX * sin(ROT_SPEED) + rc->player_dir_y * cos(ROT_SPEED);
        oldPlaneX = rc->player_plane_x;
        rc->player_plane_x = rc->player_plane_x * cos(ROT_SPEED) - rc->player_plane_y * sin(ROT_SPEED);
        rc->player_plane_y = oldPlaneX * sin(ROT_SPEED) + rc->player_plane_y * cos(ROT_SPEED);
      }
    }
  }
  return (0);
}

void          raycaster(t_raycaster *rc)
{
  SDL_bool    done;

  done = SDL_FALSE;
  while(!done)
  {
    for(int x = 0; x < WIN_X; x++)
    {
      initial_calc(rc, x);
      perform_dda(rc);
      calc_wall_height(rc);
      draw_vert_line(rc, x);
    }
    render_frame(rc);
    if (handle_events(rc) != 0)
      done = SDL_TRUE;
  }
}

int           main()
{
  t_raycaster rc;

  if (init(&rc) != 0)
    return (-1);
  raycaster(&rc);
  if (rc.renderer)
    SDL_DestroyRenderer(rc.renderer);
  if (rc.window)
    SDL_DestroyWindow(rc.window);
  SDL_Quit();
  return (0);
}